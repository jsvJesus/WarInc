using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    public class DBItemUpgradeData : SQLRec
    {
        public SQLField ItemID = new SQLField("ItemID");
        public SQLField Category = new SQLField("Category");
        public SQLField Name = new SQLField("Name");
        public SQLField Description = new SQLField("Description");
        public SQLField UpgradeID = new SQLField("UpgradeID");
        public SQLField GPPrice = new SQLField("PriceP");
        public SQLField GPChance = new SQLField("GPChance");
        public SQLField GDPrice = new SQLField("GPriceP");
        public SQLField GDChance = new SQLField("GDChance");
        public SQLField Value = new SQLField("Value");

        public static string GET_TABLE_NAME()
        {
            return "Items_UpgradeData";
        }

        public DBItemUpgradeData()
        {
            TABLE_ID = GET_TABLE_NAME();

            RegisterField(ItemID);
            RegisterField(Category);
            RegisterField(Name);
            RegisterField(Description);
            RegisterField(UpgradeID);
            RegisterField(GPPrice);
            RegisterField(GPChance);
            RegisterField(GDPrice);
            RegisterField(GDChance);
            RegisterField(Value);
        }

        public void SetNewRecordDefaults(int UpgradeID, int Level)
        {
        }
    }
}
