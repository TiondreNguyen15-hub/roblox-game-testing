# cuDNN first-use crash fix

## PC evidence and current result

The user's Windows Application event identified `cudnn64_9.dll` **9.8.0.87**, exception **0xc0000409**, while the app was entering ONNX CUDA inference. The app log showed successful CUDA vector-add and CPU inference, with no completed GPU result. The code is a general fail-fast status; it does not by itself prove a buffer-overrun bug or exact cause.

With the app's private `runtime/gpu` folder temporarily prepended to the launching PowerShell process's PATH, the user then reported:

- RTX 3070 CUDA vector-add: **passed**, full reference readback verified.
- CPU ONNX inference: **passed**, numerical reference agreed.
- ONNX CUDA EP inference: **passed**, actual CUDA compute-node profile and numerical agreement, CPU EP fallback disabled, TF32 off.

This establishes a successful GPU-inference run **as recorded by the user's app**, and strongly supports the private-DLL-discovery diagnosis. It does not establish sustained performance or leak freedom. The user did not supply latency/FPS numbers, so none are invented here.

## Loader defect and fix

The previous loader explicitly loaded `cudnn64_9.dll` but omitted the other cuDNN 9 DLLs. cuDNN lazily resolves companions by bare name; scoped `AddDllDirectory` registrations do not necessarily affect those legacy loads. Files can exist in `runtime/gpu` yet be missed on first use, causing native termination outside C++ exception handling.

The fixed loader checks x64 architecture and explicitly loads the complete pinned cuDNN family by absolute path **before** the umbrella DLL is used. All cuDNN files are selected beside the same `cudnn64_9.dll` to avoid mixing folders. The family includes graph, ops, adv, CNN, heuristic and both engines DLLs. Missing companions are rejected with a diagnostic before calling the dispatcher. The app's UI, settings, renderer, entity input and strict GPU-verification criteria remain unchanged. No permanent system PATH change is made.

Primary references:

- [Microsoft ORT 1.22 Windows preload order](https://github.com/microsoft/onnxruntime/blob/v1.22.0/onnxruntime/__init__.py): includes cuDNN companion DLLs, then the umbrella.
- [Upstream first-use cuDNN / 0xC0000409 report](https://github.com/pytorch/pytorch/issues/203660): reproduces the same loader mechanism on Windows ARM; it is supporting evidence, not an independent reproduction of this user's x64 PC.

## Update without reinstalling the GPU libraries

Close the app. The small `Apex-Visualization-CuDNN-Fix.zip` contains the patched `apex_visualizer.exe` and these instructions. Extract it **into the existing folder containing your app**, replacing the old executable. Keep your installed `runtime/gpu` folder. There is no need to download the 1.41 GiB dependency set again.

Launch the updated EXE normally from File Explorer, without using the temporary PowerShell PATH workaround. Open **System → Run CUDA + ONNX validation**. The new executable still marks GPU verification only after its numerical and CUDA-profile checks pass. Preserve the old executable if you want a rollback; settings remain in LocalAppData.

The full Windows ZIP and offline source ZIP are updated as well. A fresh full-ZIP extraction includes only the CPU runtime; copy your existing GPU runtime folder or use the installer if moving to a new directory.

## What still needs measurement

Normal-launch confirmation of the patched binary on the RTX PC, repeated/extended GPU execution, numerical stability over time, physical process-counter comparisons, CPU/GPU latency/FPS figures and native leak diagnostics remain separate tests. A successful short test proves execution, not a speedup or prolonged stability. Use the benchmark/collector tools for those records. See the accompanying `CUDNN-FIX-VALIDATION.md` for actual build-host checks, including the dispatcher first-use test and missing-companion regression.
