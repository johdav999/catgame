// Generated from verified accepted manifest records, never model job exports.
static const CatInput catInputs[] = {
{"pip","release1/cats/pip@3","assets/models/release1/cats/toon-v2/pip/pip.glb"},
{"mabel","release1/cats/mabel@3","assets/models/release1/cats/toon-v2/mabel/mabel.glb"},
{"button","release1/cats/button@3","assets/models/release1/cats/toon-v2/button/button.glb"},
{"pepper","release1/cats/pepper@3","assets/models/release1/cats/toon-v2/pepper/pepper.glb"},
};
static const PortraitInput catPortraitInputs[] = {
{"pip","assets/models/release1/cats/toon-v2/pip/portrait.png"},
{"mabel","assets/models/release1/cats/toon-v2/mabel/portrait.png"},
{"button","assets/models/release1/cats/toon-v2/button/portrait.png"},
{"pepper","assets/models/release1/cats/toon-v2/pepper/portrait.png"},
{nullptr,nullptr}};
void acceptedContacts(paws::release1::garden::Assets& assets) {
assets.cats[0].contacts["garden_cushion"]={{0.0f,0.0f,0.85f},{0.04306325576097547f,0.20085412503868755f,-0.15371721647841638f},0.0f,"settle_cushion","curled_rest","rest_leave",2.4,2.4,0.0,{},true};
assets.cats[0].naturalCurl=true;
assets.cats[0].contacts["drinking_bowl"]={{-0.0009217480700478824f,0.0f,-1.1f},{-0.0009217480700478824f,0.0f,-0.6284978173188536f},0.0f,"approach_bowl","drink","return_idle",1.8,1.8,0.0,{},true};
assets.cats[0].contacts["scratching_post"]={{0.0f,0.0f,1.1f},{0.005769380609209476f,0.0f,0.8745293597919935f},3.141592653589793f,"scratch","scratch","return_idle",3.0,1.8,0.43333333333333335,{},true};
assets.cats[0].contacts["playhouse"]={{0.0f,0.0f,1.6f},{0.0f,0.07f,0.0f},3.141592653589793f,"playhouse_enter","playhouse_lookout","playhouse_enter",1.8,1.8,0.0,{{0.0f,0.0f,1.6f},{0.0f,0.07f,0.75f},{0.0f,0.07f,0.0f}},true};
assets.cats[0].contacts["low_picnic_table"]={{-1.275f,0.0f,0.0f},{-0.85f,0.0f,0.0f},1.5707963267948966f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[0].contacts["picnic_mat"]={{-1.4f,0.0f,0.8f},{-1.4f,0.0f,0.0f},1.5707963267948966f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[1].contacts["garden_cushion"]={{0.0f,0.0f,0.85f},{0.04306325576097547f,0.20085412503868755f,-0.15371721647841638f},0.0f,"settle_cushion","curled_rest","rest_leave",2.4,2.4,0.0,{},true};
assets.cats[1].naturalCurl=true;
assets.cats[1].contacts["drinking_bowl"]={{-0.0009217480700478824f,0.0f,-1.1f},{-0.0009217480700478824f,0.0f,-0.6284978173188536f},0.0f,"approach_bowl","drink","return_idle",1.8,1.8,0.0,{},true};
assets.cats[1].contacts["sunny_perch"]={{0.0f,0.0f,1.25f},{0.0f,0.46f,0.0f},3.141592653589793f,"perch_step","sit_idle","perch_step",2.4,2.4,0.0,{{0.0f,0.0f,1.0f},{0.0f,0.2f,0.575f},{0.0f,0.46f,0.0f}},true};
assets.cats[1].contacts["low_picnic_table"]={{0.0f,0.0f,-1.4249999999999998f},{0.0f,0.0f,-0.95f},0.0f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[1].contacts["picnic_mat"]={{0.0f,0.0f,-1.7999999999999998f},{0.0f,0.0f,-1.4f},0.0f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[2].contacts["garden_cushion"]={{0.0f,0.0f,0.85f},{0.04306325576097547f,0.20085412503868755f,-0.15371721647841638f},0.0f,"settle_cushion","curled_rest","rest_leave",2.4,2.4,0.0,{},true};
assets.cats[2].naturalCurl=true;
assets.cats[2].contacts["drinking_bowl"]={{-0.0009217480700478824f,0.0f,-1.1f},{-0.0009217480700478824f,0.0f,-0.6284978173188536f},0.0f,"approach_bowl","drink","return_idle",1.8,1.8,0.0,{},true};
assets.cats[2].contacts["hammock"]={{0.95f,0.0f,0.0f},{0.04306325576097547f,0.6216716708172539f,-0.15371721647841638f},0.0f,"settle_cushion","curled_rest","rest_leave",2.4,2.4,0.0,{{0.85f,0.0f,0.0f},{0.04306325576097547f,0.6216716708172539f,-0.15371721647841638f}},true};
assets.cats[2].naturalCurl=true;
assets.cats[2].contacts["low_picnic_table"]={{1.275f,0.0f,0.0f},{0.85f,0.0f,0.0f},-1.5707963267948966f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[2].contacts["picnic_mat"]={{1.4f,0.0f,0.8f},{1.4f,0.0f,0.0f},-1.5707963267948966f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[3].contacts["garden_cushion"]={{0.0f,0.0f,0.85f},{0.04306325576097547f,0.20085412503868755f,-0.15371721647841638f},0.0f,"settle_cushion","curled_rest","rest_leave",2.4,2.4,0.0,{},true};
assets.cats[3].naturalCurl=true;
assets.cats[3].contacts["drinking_bowl"]={{-0.0009217480700478824f,0.0f,-1.1f},{-0.0009217480700478824f,0.0f,-0.6284978173188536f},0.0f,"approach_bowl","drink","return_idle",1.8,1.8,0.0,{},true};
assets.cats[3].contacts["playhouse"]={{0.0f,0.0f,1.6f},{0.0f,0.07f,0.0f},3.141592653589793f,"playhouse_enter","playhouse_lookout","playhouse_enter",1.8,1.8,0.0,{{0.0f,0.0f,1.6f},{0.0f,0.07f,0.75f},{0.0f,0.07f,0.0f}},true};
assets.cats[3].contacts["low_picnic_table"]={{0.0f,0.0f,1.4249999999999998f},{0.0f,0.0f,0.95f},3.141592653589793f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
assets.cats[3].contacts["picnic_mat"]={{0.0f,0.0f,0.9f},{0.0f,0.04f,0.1f},3.141592653589793f,"picnic_sit","sit_idle","sit_leave",2.4,1.8,0.0,{},true};
}
