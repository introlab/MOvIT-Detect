from tests.BaseBackendTest import BaseBackendTest
import unittest
from requests import get, post, delete


class ConfigEndpointTest(BaseBackendTest):

    def setUp(self):
        answer = self._login_with_http_auth('clinician', 'movit-admin')
        self.assertEqual(answer.status_code, 200)
        self.token = answer.json()['token']
        self.assertGreater(len(self.token), 0)

    def tearDown(self):
        pass

    def test_configuration_get(self):
        answer = self._get_request_with_token_auth('/configuration')
        self.assertEqual(answer.status_code, 200)
        result = answer.json()
        self.assertTrue('Value' in result)
        self.assertEqual('Configuration', result['Value'])
        self.assertTrue('_id' in result)
        self.assertTrue('maxAngle' in result)
        self.assertTrue('minAngle' in result)

    def test_configuration_post(self):
        # TODO
        pass

    def test_data_agreement_get(self):
        # TODO
        answer = self._get_request_with_token_auth('/dataAgreement')
        self.assertEqual(answer.status_code, 200)
        result = answer.json()
        self.assertTrue('Value' in result)
        self.assertEqual('DataAgreement', result['Value'])
        self.assertTrue('_id' in result)
        self.assertTrue('dataAgreement' in result)

    def test_data_agreement_post(self):
        # TODO
        pass

