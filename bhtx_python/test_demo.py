import unittest
from bhtx import *

#### It is one of the unit test demo to be able to add more test cases here
#### test module inherits unittest.TestCase
class test(unittest.TestCase):

    bhtx = None

    @classmethod
    def setUpClass(cls):
        cls.bhtx = BhTx.get_instance()

    def testTxtype(self):  # example: test case 1
        self.assertTrue(TxType.BASIC_PLUGON == self.bhtx.get_type())
    
    def testTxRfVariant(self): # example : test case 2
        self.assertTrue(RfVariant.A == self.bhtx.get_variant())


