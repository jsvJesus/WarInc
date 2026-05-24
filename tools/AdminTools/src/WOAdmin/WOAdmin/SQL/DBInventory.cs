using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
    * * */

    public class DBInventory : SQLRec
    {
        public SQLField CustomerID  = new SQLField("CustomerID");
        public SQLField ItemID = new SQLField("ItemID");
        public SQLField LeasedUntil = new SQLField("LeasedUntil");

        public static string GET_TABLE_NAME()
        {
            return "Inventory";
        }

        public DBInventory()
        {
            TABLE_ID = GET_TABLE_NAME();
            //RECORD_ID = new SQLField("NO_RECORD_ID");

            RegisterField(CustomerID);
            RegisterField(ItemID);
            RegisterField(LeasedUntil);
        }
    }
}
