using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    public class DBLootEntry : SQLRec
    {
        public SQLField LootID = new SQLField("LootID");
        public SQLField Chance = new SQLField("Chance");
        public SQLField ItemID = new SQLField("ItemID");
        public SQLField ExpDaysMin = new SQLField("ExpDaysMin");
        public SQLField ExpDaysMax = new SQLField("ExpDaysMax");
        public SQLField GDMin = new SQLField("GDMin");
        public SQLField GDMax = new SQLField("GDMax");
        public SQLField GDIfHave = new SQLField("GDIfHave");

        public static string GET_TABLE_NAME()
        {
            return "Items_LootData";
        }

        public DBLootEntry()
        {
            TABLE_ID = GET_TABLE_NAME();

            RECORD_ID = new SQLField("RecordID");

            RegisterField(LootID);
            RegisterField(Chance);
            RegisterField(ItemID);
            RegisterField(ExpDaysMin);
            RegisterField(ExpDaysMax);
            RegisterField(GDMin);
            RegisterField(GDMax);
            RegisterField(GDIfHave);
        }

        public void SetNewRecordDefaults()
        {
            LootID.FromObject(0);
            Chance.FromObject(100.0f);
            ItemID.FromObject(0);
            ExpDaysMin.FromObject(1);
            ExpDaysMax.FromObject(1);
            GDMin.FromObject(0);
            GDMax.FromObject(0);
            GDIfHave.FromObject(0);
        }
    }
}
