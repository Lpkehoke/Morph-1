import unittest

import _test as test


class TestStringMethods(unittest.TestCase):

    def setUp(self):
        self.test_return_values = test.TestReturnValues()

    def test_return_cpp_int(self):
        res = self.test_return_values.get_one()
        self.assertEqual(res, 1)
