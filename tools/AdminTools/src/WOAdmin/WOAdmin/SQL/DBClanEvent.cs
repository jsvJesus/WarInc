using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    public class DBClanEvent : SQLRec
    {
        public static EditField_Combo.SPairs EventTypeValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("1", "Created"), 
            new EditField_Combo.SPair("3", "SetRank"), 
            new EditField_Combo.SPair("4", "Joined"), 
            new EditField_Combo.SPair("5", "Left"), 
            new EditField_Combo.SPair("6", "Kicked"), 
            new EditField_Combo.SPair("10", "GP to Clan"), 
            new EditField_Combo.SPair("11", "GP to Member"), 
            new EditField_Combo.SPair("12", "LevelUp"), 
            new EditField_Combo.SPair("13", "AddMaxMembers"), 
        };

        public SQLField ClanID = new SQLField("ClanID");
        public SQLField EventDate = new SQLField("EventDate");
        public SQLField EventType = new SQLField("EventType");
        public SQLField EventRank = new SQLField("EventRank");
        public SQLField Var1 = new SQLField("Var1");
        public SQLField Var2 = new SQLField("Var2");
        public SQLField Var3 = new SQLField("Var3");
        public SQLField Var4 = new SQLField("Var4");
        public SQLField Text1 = new SQLField("Text1");
        public SQLField Text2 = new SQLField("Text2");
        public SQLField Text3 = new SQLField("Text3");

        public static string GET_TABLE_NAME()
        {
            return "ClanEvents";
        }

        public DBClanEvent()
        {
            TABLE_ID = GET_TABLE_NAME();
            RECORD_ID = null; // new SQLField("ClanEventID");

            RegisterField(ClanID);
            RegisterField(EventDate);
            RegisterField(EventType);
            RegisterField(EventRank);
            RegisterField(Var1);
            RegisterField(Var2);
            RegisterField(Var3);
            RegisterField(Var4);
            RegisterField(Text1);
            RegisterField(Text2);
            RegisterField(Text3);
        }
    }
}
