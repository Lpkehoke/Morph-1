import unittest

import _test as test


class TestReturnValues(unittest.TestCase):

    def setUp(self):
        self.test_return_values = test.TestReturnValues()
        self.inspector = test.InternalsInspector()

    def assert_inst_count(self, expected_inst_count):
        inst_count = self.inspector.instances_count()
        self.assertEqual(
            expected_inst_count,
            inst_count,
            f"Registered instances count doesn't match expected one\n"
            f"Registered instances dump:\n"
            f"{self.inspector.dump_instances()}")

    def test_return_cpp_int(self):
        res = self.test_return_values.get_one_int()
        self.assertEqual(1, res)
        self.assert_inst_count(2)

    def test_return_cpp_string(self):
        res = self.test_return_values.get_hello_string()
        self.assertEqual("hello", res)
        self.assert_inst_count(2)

    def test_return_cpp_nocopyable_ref(self):
        res1 = self.test_return_values.get_nocopyable_ref()
        res2 = self.test_return_values.get_nocopyable_ref()
        self.assertTrue(res1 is res2)
        self.assert_inst_count(3)
        del res1
        del res2
        self.assert_inst_count(2)

    def test_return_cpp_nocopyable(self):
        res1 = self.test_return_values.get_nocopyable()
        res2 = self.test_return_values.get_nocopyable()
        self.assertFalse(res1 is res2)
        self.assert_inst_count(4)
        del res1
        del res2
        self.assert_inst_count(2)


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
            def say_abstract(self):
                return "abstract_hello"

        d = Derived()
        self.assertEqual("hello", d.say_hello())
        self.assertEqual("abstract_hello", d.say_abstract())
        self.assertEqual("abstract_hello", d.call_say_abstract())
