import importlib.util
import unittest
from pathlib import Path
spec = importlib.util.spec_from_file_location('report_audit', Path(__file__).resolve().parents[1] / 'tools' / 'analyze-compute-report.py')
audit_module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(audit_module)

class AuditTests(unittest.TestCase):
    def test_missing_data_never_passes(self):
        r = audit_module.audit({})
        self.assertTrue(all(x['classification'] != 'passed' for x in r['tests']))
    def test_skipped_gpu_is_not_failed_execution(self):
        r = audit_module.audit({'cuda_ep_inference': {'state': 'unavailable', 'verified': False, 'message': 'CUDA workload not verified; test not started'}})
        self.assertEqual(r['tests'][2]['recorded_state'], 'unavailable')
        self.assertEqual(r['tests'][2]['classification'], 'skipped')
    def test_pass_without_measurement_is_inconsistent(self):
        r = audit_module.audit({'cpu_inference': {'state': 'passed', 'verified': True}})
        self.assertEqual(r['tests'][1]['classification'], 'inconsistent')
    def test_software_adapter_is_not_rtx_evidence(self):
        self.assertFalse(audit_module.audit({'adapter': 'Microsoft Basic Render Driver'})['targets_rtx_3070_as_recorded'])
    def test_adapter_name_does_not_verify_execution(self):
        r = audit_module.audit({'adapter': 'NVIDIA GeForce RTX 3070', 'cuda_ep_inference': {'state': 'failed', 'verified': False}})
        self.assertTrue(r['targets_rtx_3070_as_recorded'])
        self.assertEqual(r['tests'][2]['classification'], 'failed')
    def test_requested_duration_is_not_completed_soak(self):
        r = audit_module.audit({'minimum_seconds_per_available_phase': 600})
        self.assertEqual(next(x['classification'] for x in r['tests'] if x['test'] == 'sustained_cuda_inference'), 'incomplete')

if __name__ == '__main__':
    unittest.main()
