#include "quest_adapter.hpp"
#include <iomanip>
#include <sstream>
#include <set>

namespace paws::release1::progression {
namespace q=engine::quests;
namespace {
std::string completedKey(std::string_view id){return std::string(id)+".completed";}
std::string receipt(std::string_view id){return std::string(id)+"/first-completion";}
std::shared_ptr<const q::Catalog> catalog() {
    static const auto value=[] {
        engine::ModuleComposition<q::Bindings> composition;
        engine::ModuleDescriptor descriptor;descriptor.id="paws.quests";
        descriptor.dependencies.push_back({"webengine.quests",{}});
        // Application bindings consume only validated immutable puzzle facts.
        // The engine remains independent of Paws move legality and economy.
        descriptor.schemas.push_back({"paws.puzzle-facts",{1,0,0}});
        engine::ModuleRegistration<q::Bindings> pawsModule{descriptor,[](q::Bindings&) {}};
        std::vector<engine::ModuleRegistration<q::Bindings>> modules{q::moduleRegistration(),pawsModule};
        auto bindings=composition.replace(modules);
        q::CatalogDefinition definition;definition.id="paws.quests";definition.content="paws.catalogue.v1";
        definition.facts.emplace("beach_fresh_water.checkpoint",false);
        for(const auto& policy:quests) {
            definition.facts.emplace(completedKey(policy.id),false);
            q::QuestDefinition quest;quest.id=policy.id;quest.content=policy.content;quest.entry="puzzle";
            for(auto prerequisite:policy.prerequisites)quest.prerequisites.emplace_back(prerequisite);
            q::Stage puzzle;puzzle.id="puzzle";
            puzzle.objectives.push_back({"solved",q::ObjectiveMode::Current,{"quest.fact",1,
                {{"key",completedKey(policy.id)},{"value",true}}}});
            if(policy.id=="beach_fresh_water") {
                q::Stage basin;basin.id="basin";
                basin.objectives.push_back({"filled",q::ObjectiveMode::Current,{"quest.fact",1,
                    {{"key",std::string("beach_fresh_water.checkpoint")},{"value",true}}}});
                basin.transitions.push_back({{"quest.always",1,{}},"puzzle"});
                quest.entry="basin";quest.stages.push_back(std::move(basin));
            }
            quest.stages.push_back(std::move(puzzle));
            quest.effects.push_back({"first-completion","paws.completion",{}});
            definition.quests.push_back(std::move(quest));
        }
        return q::Catalog::create(std::move(definition),std::move(bindings));
    }();
    return value;
}
void require(bool ok,const char* why){if(!ok)throw std::invalid_argument(why);}
}
QuestAdapter::QuestAdapter():runtime_(catalog()) {}
std::shared_ptr<const QuestAdapter> QuestAdapter::reconcile(const QuestFacts& facts,
    const std::shared_ptr<const QuestAdapter>& previous,bool replay) {
    auto next=std::shared_ptr<QuestAdapter>(new QuestAdapter);
    if(previous)next->runtime_.restore(previous->runtime_.state());
    auto apply=[&](q::Batch batch) {
        batch.replay=replay;
        auto prepared=next->runtime_.prepare(batch);
        require(prepared.status==q::PrepareStatus::Ready&&prepared.transaction.has_value(),"Quest comparison budget exceeded");
        for(const auto& effect:prepared.transaction->effects()) {
            require(effect.receipt==receipt(effect.quest),"Unexpected quest receipt");
            require(effect.type=="paws.completion","Unexpected quest effect");
            next->diagnostics_.newReceipts.push_back(effect.receipt);
        }
        require(next->runtime_.commit(std::move(*prepared.transaction)),"Quest candidate became stale");
    };
    auto settle=[&](bool seed) {
        auto batch=next->runtime_.batch();
        for(std::size_t n=0;n<facts.size();++n) {
            const auto& fact=facts[n];require(fact.id==quests[n].id,"Unknown/out-of-order puzzle fact");
            batch.facts[completedKey(fact.id)]=seed?fact.earned:fact.completed;
            if(fact.id=="beach_fresh_water")batch.facts["beach_fresh_water.checkpoint"]=seed?fact.earned:fact.checkpoint;
            const auto& progress=next->runtime_.view(fact.id);
            const bool reset=(progress.status==q::Status::Completed&&!fact.completed)||
                (fact.id=="beach_fresh_water"&&progress.stage=="puzzle"&&!fact.checkpoint);
            if(!seed&&reset)batch.commands.push_back({fact.id,q::CommandKind::Retry,progress.activation,{}});
        }
        apply(std::move(batch));
        // Each pass unlocks at least one prerequisite layer, bounded by the DAG.
        for(std::size_t pass=0;pass<=questCount;++pass) {
            auto activate=next->runtime_.batch();
            for(const auto& fact:facts){const auto& p=next->runtime_.view(fact.id);
                if(p.status==q::Status::Available)activate.commands.push_back({fact.id,q::CommandKind::Activate,p.activation,{}});}
            if(activate.commands.empty())return;
            apply(std::move(activate));
        }
        throw std::invalid_argument("Quest prerequisite settling did not converge");
    };
    if(!previous) {
        // Reconstruct durable earned completion from already replay-validated
        // journals. This cannot grant effects or replay a reward presentation.
        const bool requestedReplay=replay;replay=true;settle(true);replay=requestedReplay;
    }
    settle(false);
    std::set<std::string> expectedReceipts;
    for(const auto& fact:facts) {
        const auto& p=next->runtime_.view(fact.id);
        const bool earned=fact.earned||fact.completed;
        const bool available=next->available(fact.id);
        require(available||!earned,"Completed quest has unearned prerequisites");
        require(p.earned==earned&&(p.status==q::Status::Completed)==fact.completed,"Quest completion differs from puzzle journal");
        if(earned)expectedReceipts.insert(receipt(fact.id));
        if(fact.id=="beach_fresh_water"&&available&&!fact.completed)
            require(p.stage==(fact.checkpoint?"puzzle":"basin"),"Water checkpoint differs from puzzle journal");
    }
    require(next->runtime_.state().receipts==expectedReceipts,"Quest receipts differ from reward ledger");
    next->diagnostics_.comparisons=previous?previous->diagnostics_.comparisons+1:1;
    for(const auto& fact:facts)next->diagnostics_.stages[fact.id]=next->runtime_.view(fact.id).stage;
    return next;
}
std::string QuestAdapter::projectionRecord(const QuestFact& f) const {
    const auto& p=runtime_.view(f.id);std::ostringstream out;
    out<<std::quoted(f.id)<<' '<<std::quoted(f.content)<<' '<<f.attempt<<' '
           <<p.earned<<' '<<(p.status==q::Status::Completed)<<' '<<f.checkpoint<<' '
           <<std::quoted(p.stage)<<' '<<std::quoted(p.earned?receipt(f.id):std::string{})<<'\n';
    return out.str();
}
std::string QuestAdapter::saveSection(const QuestFacts& facts) const {
    std::ostringstream out;out<<"QUEST_KERNEL 2 "<<facts.size()<<'\n';
    for(const auto& fact:facts)out<<projectionRecord(fact);
    return out.str();
}
}
