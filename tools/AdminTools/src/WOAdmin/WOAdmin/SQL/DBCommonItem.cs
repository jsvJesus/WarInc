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

    // this is base class for all items in game
    //  weapons, gears, and usable items should be derived from this one
    abstract public class DBCommonItem : SQLRec
    {
        public SQLField FNAME = new SQLField("FNAME");
        public SQLField Category = new SQLField("Category");
        public SQLField Name = new SQLField("Name");
        public SQLField Description = new SQLField("Description");
        public SQLField Price1 = new SQLField("Price1");
        public SQLField Price7 = new SQLField("Price7");
        public SQLField Price30 = new SQLField("Price30");
        public SQLField PriceP = new SQLField("PriceP");
        public SQLField GPrice1 = new SQLField("GPrice1");
        public SQLField GPrice7 = new SQLField("GPrice7");
        public SQLField GPrice30 = new SQLField("GPrice30");
        public SQLField GPriceP = new SQLField("GPriceP");
        public SQLField IsNew = new SQLField("IsNew");
        public SQLField LevelRequired = new SQLField("LevelRequired");

        protected DBCommonItem()
        {
            RECORD_ID = new SQLField("ItemID");

            RegisterField(FNAME);
            RegisterField(Description);
            RegisterField(Name);
            RegisterField(Category);
            RegisterField(Price1);
            RegisterField(Price7);
            RegisterField(Price30);
            RegisterField(PriceP);
            RegisterField(GPrice1);
            RegisterField(GPrice7);
            RegisterField(GPrice30);
            RegisterField(GPriceP);
            RegisterField(IsNew);
            RegisterField(LevelRequired);

            // setup varchar values
            FNAME.VARCHAR_LEN = 32;
            Name.VARCHAR_LEN = 32;
            Description.VARCHAR_LEN = 512;
        }

        protected void SetNewItemDefaults(int category)
        {
            FNAME.FromObject("");
            Category.FromObject(category.ToString());
            Name.FromObject("");
            Description.FromObject("");

            Price1.FromObject(0);
            Price7.FromObject(0);
            Price30.FromObject(0);
            PriceP.FromObject(0);
            GPrice1.FromObject(0);
            GPrice7.FromObject(0);
            GPrice30.FromObject(0);
            GPriceP.FromObject(0);
            IsNew.FromObject(0);
            LevelRequired.FromObject(0);
        }
    }
}
