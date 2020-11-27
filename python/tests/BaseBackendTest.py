import unittest
from requests import get, post, delete


class BaseBackendTest(unittest.TestCase):
    host = '192.168.3.214'
    port = 1880
    login_endpoint = '/login'
    token = None

    def _make_url(self, hostname, port, endpoint):
        return 'http://' + hostname + ':' + str(port) + endpoint

    def _login_with_http_auth(self, username, password):
        body = {'username': username, 'password': password}
        url = self._make_url(self.host, self.port, self.login_endpoint)
        return post(url=url, verify=False, auth=(username, password), data=body)

    def _get_request_with_token_auth(self, endpoint, params=None, data=None):
        if params is None:
            params = {}
        request_headers = {'authorization': self.token}
        url = self._make_url(self.host, self.port, endpoint)
        return get(url=url, verify=False, params=params, headers=request_headers, data=data)

    def _post_request_with_token_auth(self, endpoint, params=None, data=None):
        if params is None:
            params = {}
        request_headers = {'authorization': self.token}
        url = self._make_url(self.host, self.port, endpoint)
        return post(url=url, verify=False, params=params, headers=request_headers, data=data)

