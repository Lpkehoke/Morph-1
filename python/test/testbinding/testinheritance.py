import unittest

import _test as test


class TestInheritance(unittest.TestCase):

    def setUp(self):
        self.inspector = test.InternalsInspector()

    def assert_inst_count(self, expected_inst_count):
        inst_count = self.inspector.instances_count()
        self.assertEqual(
            expected_inst_count,
            inst_count,
            f"Registered instances count doesn't match expected one\n"
            f"Registered instances dump:\n"
            f"{self.inspector.dump_instances()}")

    def test_multiple_inheritance(self):
        class Derived(test.DummyA, test.DummyB, test.DummyC, test.DummyD):
            def __init__(self):
                test.DummyA.__init__(self)
                test.DummyB.__init__(self)
                test.DummyC.__init__(self)
                test.DummyD.__init__(self)

        d = Derived()
        self.assertEqual("A", d.say_a())
        self.assertEqual("B", d.say_b())
        self.assertEqual("C", d.say_c())
        self.assertEqual("D", d.say_d())

        self.assert_inst_count(2)
        del d
        self.assert_inst_count(1)

    def test_diamond_inheritance(self):
        class DerivedA(test.DummyA):
            pass

        class DerivedB(test.DummyB, DerivedA):
            pass

        class DerivedC(test.DummyC, DerivedA):
            pass

        class DerivedD(test.DummyD, DerivedB, DerivedC):
            def __init__(self):
                test.DummyA.__init__(self)
                test.DummyB.__init__(self)
                test.DummyC.__init__(self)
                test.DummyD.__init__(self)

        d = DerivedD()
        self.assertEqual("A", d.say_a())
        self.assertEqual("B", d.say_b())
        self.assertEqual("C", d.say_c())
        self.assertEqual("D", d.say_d())
        self.assert_inst_count(2)
        del d
        self.assert_inst_count(1)
