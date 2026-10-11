"""Summarize retained soak samples separately from a corrected package rerun."""
import argparse
import hashlib
import json
import re
from pathlib import Path


def read(path):
    return json.loads(path.read_text())


def distribution(values):
    values = sorted(v for v in values if isinstance(v, (int, float)) and v > 0)
    if not values:
        return {"samples": 0}
    return {"samples": len(values), "mean": sum(values) / len(values),
            **{f"p{q}": values[min(len(values)-1, int(len(values)*q/100))]
               for q in (50, 95, 99)}, "max": values[-1]}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--soak", type=Path, required=True)
    parser.add_argument("--rerun", type=Path, required=True)
    args = parser.parse_args()
    source = args.soak / "review/performance-samples.json"
    raw = read(source)
    old = read(args.soak / "journey/report.json")["runtimeBuild"]
    reports = {name: read(args.rerun / name / "report.json")
               for name in ("journey", "recovery", "review")}
    assert all(r.get("passed") for r in reports.values()), "Corrected rerun must pass"
    current = reports["review"]["build"]
    assert all(old[k] == current[k] for k in ("wasmSha256", "contentSha256", "enginePin"))
    progress = re.findall(r"Soak seconds: (\d+) transitions: (\d+)",
                          (args.soak / "full-run.log").read_text())
    seconds, cycles = map(int, progress[-1])
    samples = raw["samples"]
    counters = [s["perf"]["current"] for s in samples]
    gpu_bytes = lambda c: c["gpuPayloadBytes"] + c["guiTextureBytes"] + c["guiBufferBytes"]
    # Select two warm samples from the same garden/profile during the logged soak.
    warm = [s for s in samples if s["time"] >= samples[-1]["time"] - seconds*1000 + 5000
            and s["scene"] == "Pip's courtyard" and s["perf"]["current"]["width"] == 1214]
    memory = lambda s: {"wasmCapacityBytes": s["wasm"], "trackedGpuBytes": gpu_bytes(s["perf"]["current"]),
                        "cpuAssetBytes": s["perf"]["current"]["cpuAssetBytes"],
                        "cachedMeshes": s["perf"]["current"]["cachedMeshes"],
                        "cachedTextures": s["perf"]["current"]["cachedTextures"]}
    result = {"build": current, "correctedPackageReports": {n: str(args.rerun / n / "report.json") for n in reports},
              "correctedPackagePassed": True, "correctedPackageSoakSeconds": reports["review"]["soakSeconds"],
              "retainedSoak": {"build": old, "loggedSeconds": seconds, "gardenPuzzleCycles": cycles,
                  "rawSamples": str(source), "rawSha256": hashlib.sha256(source.read_bytes()).hexdigest(),
                  "rendererSamples": len(samples), "rafIntervals": distribution([f["ms"] for f in raw["frames"]]),
                  "byScene": {scene: distribution([f["ms"] for f in raw["frames"] if f["scene"] == scene])
                              for scene in sorted({f["scene"] for f in raw["frames"]})},
                  "sampledWindowMeanCpuUpdateMs": distribution([s["perf"]["timings"]["updateMs"]["mean"] for s in samples]),
                  "sampledWindowMeanGpuMs": distribution([s["perf"]["gpu"]["totalMs"]["mean"] for s in samples]),
                  "gpuTimingStates": sorted({s["perf"]["gpu"]["state"] for s in samples}),
                  "warmGardenBefore": memory(warm[0]), "warmGardenAfter": memory(warm[-1]),
                  "warmGardenRetainedAllocationsStable": memory(warm[0]) == memory(warm[-1]),
                  "peaks": {"wasmCapacityBytes": max(s["wasm"] for s in samples),
                      "trackedGpuBytes": max(map(gpu_bytes, counters)),
                      "cpuAssetBytes": max(c["cpuAssetBytes"] for c in counters),
                      "colorDraws": max(c["drawCalls"]-c["shadowDrawCalls"]-c["guiDrawCalls"] for c in counters),
                      "colorTriangles": max(c["triangles"]-c["shadowTriangles"]-c["guiTriangles"] for c in counters),
                      "audioPcmBytes": max(s.get("audio", {}).get("peakPcmBytes", 0) for s in samples)},
                  "terminalReportPassed": False,
                  "limitation": "Original full run completed the soak and saved raw samples, but its terminal report failed on BigInt serialization. It has no recoverable terminal pass assertion. A separate short rerun exposed a missing-WebGPU initialization error; the startup gate and serializer were corrected, and all checks rerun on the new package. Wasm, content and engine are identical. This is not a 900-second rerun of the final shell."},
              "finalPackageMeasurements": {k: reports["review"][k] for k in ("browser", "adapter", "startup", "peaks", "budgets", "limits")}}
    (args.rerun / "verification.json").write_text(json.dumps(result, indent=2)+"\n")
    print(json.dumps({"build": current["buildId"], "passed": True, "retainedSoak": result["retainedSoak"]}, indent=2))


if __name__ == "__main__":
    main()
