from tests.BaseBackendTest import BaseBackendTest
import unittest
from requests import get, post, delete


class LoginEndpointTest(BaseBackendTest):

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_login_with_http_auth_user(self):
        response = self._login_with_http_auth('user', 'movit-user')
        self.assertEqual(response.status_code, 200)
        result = response.json()
        self.assertTrue('result' in result)
        self.assertTrue('token' in result)
        self.assertEqual(result['result'], 0)
        self.assertGreater(len(result['token']), 0)

    def test_login_with_http_auth_clinician(self):
        response = self._login_with_http_auth('clinician', 'movit-admin')
        self.assertEqual(response.status_code, 200)
        result = response.json()
        self.assertTrue('result' in result)
        self.assertTrue('token' in result)
        self.assertEqual(result['result'], 0)
        self.assertGreater(len(result['token']), 0)

    def test_login_with_http_auth_clinician_bad_password(self):
        response = self._login_with_http_auth('clinician', 'invalid')
        self.assertEqual(response.status_code, 200)
        result = response.json()
        self.assertTrue('result' in result)
        self.assertFalse('token' in result)
        self.assertEqual(result['result'], 401)

    def test_login_with_http_auth_user_bad_password(self):
        response = self._login_with_http_auth('user', 'invalid')
        self.assertEqual(response.status_code, 200)
        result = response.json()
        self.assertTrue('result' in result)
        self.assertFalse('token' in result)
        self.assertEqual(result['result'], 401)

    def test_login_with_http_auth_invalid_user(self):
        response = self._login_with_http_auth('invalid', 'invalid')
        self.assertEqual(response.status_code, 401)

