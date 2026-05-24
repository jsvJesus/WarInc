using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
/*
ItemID	int	Unchecked
FNAME	varchar(64)	Unchecked
Name	varchar(32)	Unchecked
Description	varchar(256)	Unchecked
     * * */

    public class DBPackage : DBCommonItem
    {
        public static bool IS_THIS_CATEGORY(int category)
        {
            return category == 9;
        }

        public static EditField_Combo.SPairs CategoryValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("9", "Package"), 
        };

        public SQLField IsEnabled = new SQLField("IsEnabled");
        public SQLField AddGP = new SQLField("AddGP");
        public SQLField AddSP = new SQLField("AddSP");
        public SQLField Item1_ID = new SQLField("Item1_ID");
        public SQLField Item1_Exp = new SQLField("Item1_Exp");
        public SQLField Item2_ID = new SQLField("Item2_ID");
        public SQLField Item2_Exp = new SQLField("Item2_Exp");
        public SQLField Item3_ID = new SQLField("Item3_ID");
        public SQLField Item3_Exp = new SQLField("Item3_Exp");
        public SQLField Item4_ID = new SQLField("Item4_ID");
        public SQLField Item4_Exp = new SQLField("Item4_Exp");
        public SQLField Item5_ID = new SQLField("Item5_ID");
        public SQLField Item5_Exp = new SQLField("Item5_Exp");
        public SQLField Item6_ID = new SQLField("Item6_ID");
        public SQLField Item6_Exp = new SQLField("Item6_Exp");


        public static string GET_TABLE_NAME()
        {
            return "Items_Packages";
        }

        public string GetDisplayName()
        {
            return Name.ToString() + " (" + CategoryValues.GetNameByType(Category.ToString()) + ")";
        }

        public DBPackage()
        {
            TABLE_ID = GET_TABLE_NAME();

            RegisterField(IsEnabled);
            RegisterField(AddGP);
            RegisterField(AddSP);
            RegisterField(Item1_ID);
            RegisterField(Item1_Exp);
            RegisterField(Item2_ID);
            RegisterField(Item2_Exp);
            RegisterField(Item3_ID);
            RegisterField(Item3_Exp);
            RegisterField(Item4_ID);
            RegisterField(Item4_Exp);
            RegisterField(Item5_ID);
            RegisterField(Item5_Exp);
            RegisterField(Item6_ID);
            RegisterField(Item6_Exp);
        }

        public void SetNewRecordDefaults(int category)
        {
            SetNewItemDefaults(category);

            IsEnabled.FromObject(0);
            AddGP.FromObject(0);
            AddSP.FromObject(0);
            Item1_ID.FromObject(0);
            Item1_Exp.FromObject(0);
            Item2_ID.FromObject(0);
            Item2_Exp.FromObject(0);
            Item3_ID.FromObject(0);
            Item3_Exp.FromObject(0);
            Item4_ID.FromObject(0);
            Item4_Exp.FromObject(0);
            Item5_ID.FromObject(0);
            Item5_Exp.FromObject(0);
            Item6_ID.FromObject(0);
            Item6_Exp.FromObject(0);
        }
    }
}
