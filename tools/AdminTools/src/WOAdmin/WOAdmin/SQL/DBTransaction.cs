using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
/*
    CustomerID	int	Unchecked
    TransactionID	varchar(32)	Unchecked
    TransactionType	int	Unchecked
    DateTime	datetime	Unchecked
    Amount	float	Unchecked
    ResponseCode	varchar(64)	Unchecked
    AprovalCode	varchar(16)	Unchecked
    ItemID	varchar(32)	Unchecked
* * */

    public class DBTransactionRO : SQLRec
    {
        static public EditField_Combo.SPairs TransactionTypeValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("0", "-ALL-"), 
            new EditField_Combo.SPair("1000", "Real $"), 
            new EditField_Combo.SPair("2000", "GP Rent"), 
            new EditField_Combo.SPair("3000", "GP Buy"), 
            new EditField_Combo.SPair("2001", "GD Rent"), 
            new EditField_Combo.SPair("3001", "GD Buy"), 
        };

        public SQLField CustomerID = new SQLField("CustomerID");
        public SQLField TransactionID = new SQLField("TransactionID");
        public SQLField TransactionType = new SQLField("TransactionType");
        public SQLField DateTime = new SQLField("DateTime");
        public SQLField Amount = new SQLField("Amount");
        public SQLField ResponseCode = new SQLField("ResponseCode");
        public SQLField AprovalCode = new SQLField("AprovalCode");
        public SQLField ItemID = new SQLField("ItemID");

        public static string GET_TABLE_NAME()
        {
            return "FinancialTransactions";
        }

        public DBTransactionRO()
        {
            TABLE_ID = GET_TABLE_NAME();
            //@NOTE: THERE IS NO RECORD_ID YET, so this class is readonly

            RegisterField(CustomerID);
            RegisterField(TransactionID);
            RegisterField(TransactionType);
            RegisterField(DateTime);
            RegisterField(Amount);
            RegisterField(ResponseCode);
            RegisterField(AprovalCode);
            RegisterField(ItemID);
        }
    }
}
