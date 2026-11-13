"""Audit recorded results. This tool never executes or assumes a passed GPU test."""
import argparse
import json
from pathlib import Path

CORE = ('cuda_workload', 'cpu_inference', 'cuda_ep_inference')

def audit(report):
    rows = []
    for key in CORE:
        test = report.get(key, {})
        state = test.get('state', 'not_recorded')
        measured = (test.get('run_end_to_end') or {}).get('observations', 0)
        message = test.get('message', '')
        if state == 'passed' and (not test.get('verified') or not measured):
            classification = 'inconsistent'
        elif state == 'unavailable' and any(v in message.lower() for v in ('not attempted', 'not started', 'blocked')):
            classification = 'skipped'
        else:
            classification = state
        rows.append({'test': key, 'recorded_state': state, 'classification': classification,
                     'verified_as_recorded': bool(test.get('verified')), 'measured_calls': measured,
                     'reason_code': test.get('reason_code', 'not_recorded'),
                     'blocked_by': test.get('blocked_by', ''), 'evidence': message})
    comparable = all(report.get(k, {}).get('verified') and (report.get(k, {}).get('run_end_to_end') or {}).get('observations', 0) for k in ('cpu_inference', 'cuda_ep_inference'))
    rows.append({'test': 'cpu_gpu_comparison', 'classification': 'available' if comparable else 'skipped',
                 'evidence': 'Requires passed measured CPU and CUDA EP results; unavailable GPU metrics are not zero.'})
    gpu = report.get('cuda_ep_inference', {})
    duration = gpu.get('validated_loop_duration_s')
    rows.append({'test': 'sustained_cuda_inference', 'classification': 'passed_as_recorded' if gpu.get('verified') and duration is not None and duration >= 600 else 'incomplete',
                 'evidence': 'Recommended minimum: actual 600-second validated CUDA inference loop. Requested duration alone is not independent evidence.'})
    rows.append({'test': 'native_gpu_leak_analysis', 'classification': 'not_recorded',
                 'evidence': 'Before/after/peak counters and short sanitizer runs cannot prove CUDA/driver leak freedom.'})
    adapter = report.get('adapter', 'not_recorded')
    target = 'rtx3070' in ''.join(adapter.lower().split())
    return {'adapter_as_recorded': adapter, 'targets_rtx_3070_as_recorded': target,
            'provenance_note': 'Selected adapter is not recorded as RTX 3070; do not treat this as RTX compute evidence.' if not target else 'Adapter name is recorded as RTX 3070; execution still requires passed measured tests.',
            'phase_as_recorded': report.get('phase', 'not_recorded'),
            'runtime_path': report.get('runtime_path', 'not_recorded'),
            'available_providers': report.get('available_providers', []),
            'tests': rows,
            'failed_skipped_or_incomplete': [x for x in rows if x['classification'] not in ('passed', 'passed_as_recorded', 'available')],
            'dependency_problems_as_recorded': [x for x in report.get('dependencies', []) if x.get('state') in ('missing', 'wrong_architecture', 'module_conflict', 'load_failed')],
            'limits': 'This audits supplied records, not the PC remotely. No GPU success, latency or leak-free result is synthesized. Missing UI/multiplayer/other module test evidence cannot be classified as passed or failed.'}

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('report', type=Path)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if args.report.stat().st_size > 4 * 1024 * 1024:
        parser.error('Report exceeds 4 MiB')
    result = audit(json.loads(args.report.read_text(encoding='utf-8-sig')))
    result['source_file'] = str(args.report)
    text = json.dumps(result, indent=2) + '\n'
    if args.output:
        args.output.write_text(text, encoding='utf-8')
    else:
        print(text, end='')

if __name__ == '__main__':
    main()
