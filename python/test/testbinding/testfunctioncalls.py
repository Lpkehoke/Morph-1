import unittest

import _test as test


class TestReturnValues(unittest.TestCase):

    def setUp(self):
        self.test_return_values = test.TestReturnValues()

    def test_return_cpp_int(self):
        res = self.test_return_values.get_one_int()
        self.assertEqual(res, 1)

    def test_return_cpp_nocopyable(self):
        res1 = self.test_return_values.get_nocopyable()
        res2 = self.test_return_values.get_nocopyable()
        self.assertFalse(res1 is res2)

    def test_return_cpp_nocopyable_ref(self):
        res1 = self.test_return_values.get_nocopyable_ref()
        res2 = self.test_return_values.get_nocopyable_ref()
        self.assertTrue(res1 is res2)
