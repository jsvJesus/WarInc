using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
    ItemID	int	Unchecked
    FNAME	varchar(32)	Unchecked
    Category	int	Unchecked
    Name	varchar(32)	Unchecked
    Description	varchar(256)	Unchecked
    */

    public class DBItem : DBCommonItem
    {
        public SQLField IsStackable = new SQLField("IsStackable");

        public static bool IS_THIS_CATEGORY(int category)
        {
            return category >= 0 && category <= 7;
        }

        public static EditField_Combo.SPairs CategoryValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("1", "Account"), 
            new EditField_Combo.SPair("2", "Boost"), 
            new EditField_Combo.SPair("3", "Mystery Box"), 
            new EditField_Combo.SPair("7", "Loot Box"), 
            new EditField_Combo.SPair("4", "Items"), 
            new EditField_Combo.SPair("5", "Abilities"), 
            new EditField_Combo.SPair("6", "Airstrikes"), 
        };

        public bool Is_Mystery_Crate()
        {
            if (Category.AsInt() == 3 || Category.AsInt() == 7)
                return true;
            return false;
        }

        public static string GET_TABLE_NAME()
        {
            return "Items_Generic";
        }

        public DBItem()
        {
            TABLE_ID = GET_TABLE_NAME();

            RegisterField(IsStackable);
        }

        public void SetNewRecordDefaults(int category)
        {
            SetNewItemDefaults(category);
            IsStackable.FromObject(0);
        }
    }
}
