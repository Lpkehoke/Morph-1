import unittest

import _test as test


class TestReturnValues(unittest.TestCase):

    def setUp(self):
        self.test_return_values = test.TestReturnValues()

    def test_return_cpp_int(self):
        res = self.test_return_values.get_one_int()
        self.assertEqual(res, 1)

    def test_return_cpp_string(self):
        res = self.test_return_values.get_hello_string()
        self.assertEqual(res, "hello")

    def test_return_cpp_nocopyable(self):
        res1 = self.test_return_values.get_nocopyable()
        res2 = self.test_return_values.get_nocopyable()
        self.assertFalse(res1 is res2)

    def test_return_cpp_nocopyable_ref(self):
        res1 = self.test_return_values.get_nocopyable_ref()
        res2 = self.test_return_values.get_nocopyable_ref()
        self.assertTrue(res1 is res2)


class TestParameterValues(unittest.TestCase):

    def setUp(self):
        self.test_parameter_values = test.TestParameterValues()

    def test_take_cpp_int(self):
        res = self.test_parameter_values.take_one_int(1)
        self.assertTrue(res)

    def test_take_cpp_string(self):
        res = self.test_parameter_values.take_hello_string("hello")
        self.assertTrue(res)

    def test_take_cpp_nocopyable_ref(self):
        nc = test.Nocopyable()

        # Sets nc value to "bar"
        res = self.test_parameter_values.take_nocopyable_ref(nc)

        self.assertTrue(res)
        self.assertEqual("bar", nc.foo())

    def test_take_cpp_nocopyable_shared_ptr(self):
        nc = test.Nocopyable()

        # Sets nc value to "bar"
        res = self.test_parameter_values.take_nocopyable_shared_ptr(nc)

        self.assertTrue(res)
        self.assertEqual("bar", nc.foo())


class TestAbstractMethodCalls(unittest.TestCase):

    def test_abstract_class_is_not_constructible(self):
        self.assertRaises(TypeError, test.AbstractClass)

    def test_derived_abstract_class_is_not_constructible(self):
        class Derived(test.AbstractClass):
            pass

        self.assertRaises(TypeError, Derived)

    def test_derived_class_is_constructible(self):
        class Derived(test.AbstractClass):
            def abstract_method(self):
                pass

        d = Derived()
        self.assertEqual("hello", d.say_hello())
