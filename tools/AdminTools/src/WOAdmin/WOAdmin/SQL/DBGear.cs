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
Category	int	Unchecked
Weight	int	Unchecked
DamagePerc	int	Unchecked
DamageMax	int	Unchecked
Bulkiness	int	Unchecked
Inaccuracy	int	Unchecked
Stealth	int	Unchecked
CustomFunction	int	Unchecked
Protection	char(5)	Unchecked
ProtectionLevel int Unchecked
     * * */

    public class DBGear : DBCommonItem
    {
        public static bool IS_THIS_CATEGORY(int category)
        {
            return category >= 10 && category <= 18;
        }

        public static EditField_Combo.SPairs CategoryValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("16", "Heroes"), 
            new EditField_Combo.SPair("10", "Characters"), 
            new EditField_Combo.SPair("11", "Gear"), 
            new EditField_Combo.SPair("12", "Heads"), 
            new EditField_Combo.SPair("13", "HeadGear"), 
            new EditField_Combo.SPair("14", "Camo"), 
            new EditField_Combo.SPair("15", "Voice"), 
        };

        public SQLField Weight = new SQLField("Weight");
        public SQLField DamagePerc = new SQLField("DamagePerc");
        public SQLField DamageMax = new SQLField("DamageMax");
        public SQLField Bulkiness = new SQLField("Bulkiness");
        public SQLField Inaccuracy = new SQLField("Inaccuracy");
        public SQLField Stealth = new SQLField("Stealth");
        public SQLField CustomFunction = new SQLField("CustomFunction");
        public SQLField Protection = new SQLField("Protection");
        public SQLField ProtectionLevel = new SQLField("ProtectionLevel");

        public static string GET_TABLE_NAME()
        {
            return "Items_Gear";
        }

        public string GetDisplayName()
        {
            return Name.ToString() + " (" + CategoryValues.GetNameByType(Category.ToString()) + ")";
        }

        public DBGear()
        {
            TABLE_ID = GET_TABLE_NAME();

            RegisterField(Weight);
            RegisterField(DamagePerc);
            RegisterField(DamageMax);
            RegisterField(Bulkiness);
            RegisterField(Inaccuracy);
            RegisterField(Stealth);
            RegisterField(CustomFunction);
            RegisterField(Protection);
            RegisterField(ProtectionLevel);

            // setup varchar values
            Protection.VARCHAR_LEN = 5;
        }

        public void SetNewRecordDefaults(int category)
        {
            SetNewItemDefaults(category);

            Weight.FromObject(0);
            DamagePerc.FromObject(0);
            DamageMax.FromObject(0);
            Bulkiness.FromObject(0);
            Inaccuracy.FromObject(0);
            Stealth.FromObject(0);
            CustomFunction.FromObject(0);
            Protection.FromObject("00000");
            ProtectionLevel.FromObject(1);
        }
    }
}
