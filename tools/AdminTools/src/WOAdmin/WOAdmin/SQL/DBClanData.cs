using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
/*
ClanID	int	Unchecked
ClanName	nvarchar(64)	Unchecked
ClanNameColor	int	Unchecked
ClanTag	nvarchar(4)	Unchecked
ClanTagColor	int	Unchecked
ClanEmblemID	int	Unchecked
ClanEmblemColor	int	Unchecked
ClanXP	int	Unchecked
ClanLevel	int	Unchecked
ClanGP	int	Unchecked
OwnerID	int	Unchecked
MaxClanMembers	int	Unchecked
NumClanMembers	int	Unchecked
ClanLore	nvarchar(512)	Checked
Announcement	nvarchar(512)	Checked
     * * */

    public class DBClanData : SQLRec
    {
        public SQLField ClanName = new SQLField("ClanName");
        public SQLField ClanNameColor = new SQLField("ClanNameColor");
        public SQLField ClanTag = new SQLField("ClanTag");
        public SQLField ClanTagColor = new SQLField("ClanTagColor");
        public SQLField ClanEmblemID = new SQLField("ClanEmblemID");
        public SQLField ClanEmblemColor = new SQLField("ClanEmblemColor");
        public SQLField ClanXP = new SQLField("ClanXP");
        public SQLField ClanLevel = new SQLField("ClanLevel");
        public SQLField ClanGP = new SQLField("ClanGP");
        public SQLField OwnerID = new SQLField("OwnerID");
        public SQLField MaxClanMembers = new SQLField("MaxClanMembers");
        public SQLField NumClanMembers = new SQLField("NumClanMembers");
        public SQLField ClanLore = new SQLField("ClanLore");
        public SQLField Announcement = new SQLField("Announcement");
        public SQLField ClanCreateDate = new SQLField("ClanCreateDate");

        public static string GET_TABLE_NAME()
        {
            return "ClanData";
        }

        public DBClanData()
        {
            TABLE_ID = GET_TABLE_NAME();
            RECORD_ID = new SQLField("ClanID");

            RegisterField(ClanName);
            RegisterField(ClanNameColor);
            RegisterField(ClanTag);
            RegisterField(ClanTagColor);
            RegisterField(ClanEmblemID);
            RegisterField(ClanEmblemColor);
            RegisterField(ClanXP);
            RegisterField(ClanLevel);
            RegisterField(ClanGP);
            RegisterField(OwnerID);
            RegisterField(MaxClanMembers);
            RegisterField(NumClanMembers);
            RegisterField(ClanLore);
            RegisterField(Announcement);
            RegisterField(ClanCreateDate);

            // setup varchar values
            ClanTag.VARCHAR_LEN = 4;
        }
    }
}
