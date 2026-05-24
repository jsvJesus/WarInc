using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
/*
CustomerID	int	Unchecked
AccountName	varchar(16)	Unchecked
AccountStatus	int	Unchecked
GamePoints	int	Unchecked
HonorPoints	int	Unchecked
SkillPoints	int	Unchecked
Gamertag	varchar(16)	Unchecked
dateregistered	datetime	Unchecked
lastlogindate	datetime	Unchecked
lastloginIP	varchar(16)	Unchecked
lastgamedate	datetime	Unchecked
ReferralID	int	Unchecked
* * */

    public class DBUser : SQLRec
    {
        static public EditField_Combo.SPairs AccountStatusValues = new EditField_Combo.SPairs
        {
            new EditField_Combo.SPair("100", "Registered"), 
            new EditField_Combo.SPair("101", "Active"), 
            new EditField_Combo.SPair("200", "Frozen"), 
            new EditField_Combo.SPair("300", "Fraud"), 
        };

        //public SQLField CustomerID = new SQLField("CustomerID");
        public SQLField AccountName = new SQLField("AccountName");
        public SQLField AccountStatus = new SQLField("AccountStatus");
        public SQLField GamePoints = new SQLField("GamePoints");
        public SQLField GameDollars = new SQLField("GameDollars");
        public SQLField HonorPoints = new SQLField("HonorPoints");
        public SQLField SkillPoints = new SQLField("SkillPoints");
        public SQLField Gamertag = new SQLField("Gamertag");
        public SQLField dateregistered = new SQLField("dateregistered");
        public SQLField lastlogindate = new SQLField("lastlogindate");
        public SQLField lastloginIP = new SQLField("lastloginIP");
        public SQLField lastgamedate = new SQLField("lastgamedate");
        public SQLField ReferralID = new SQLField("ReferralID");
        public SQLField IsFPSEnabled = new SQLField("IsFPSEnabled");
        public SQLField IsDeveloper = new SQLField("IsDeveloper");
        public SQLField ClanID = new SQLField("ClanID");
        public SQLField ClanRank = new SQLField("ClanRank");
        public SQLField ClanContributedGP = new SQLField("ClanContributedGP");
        public SQLField ClanContributedXP = new SQLField("ClanContributedXP");

        // reflected vars from AccountInfo
        public DBAccountInfo refl_AccountInfo = null;
        public DBStatsRO refl_Stats = null;

        public static string GET_TABLE_NAME()
        {
            return "LoginID";
        }

        public DBUser()
        {
            TABLE_ID = GET_TABLE_NAME();
            RECORD_ID = new SQLField("CustomerID");

            RegisterField(AccountName);
            RegisterField(AccountStatus);
            RegisterField(GamePoints);
            RegisterField(GameDollars);
            RegisterField(HonorPoints);
            RegisterField(SkillPoints);
            RegisterField(Gamertag);
            RegisterField(dateregistered);
            RegisterField(lastlogindate);
            RegisterField(lastloginIP);
            RegisterField(lastgamedate);
            RegisterField(ReferralID);
            RegisterField(IsFPSEnabled);
            RegisterField(IsDeveloper);
            RegisterField(ClanID);
            RegisterField(ClanRank);
            RegisterField(ClanContributedXP);
            RegisterField(ClanContributedGP);

            // setup varchar values
            AccountName.VARCHAR_LEN = 32;
            Gamertag.VARCHAR_LEN = 32;
            lastloginIP.VARCHAR_LEN = 16;
        }

        public void SetNewRecordDefaults()
        {
            AccountName.FromObject("");
            AccountStatus.FromObject(100);
            GamePoints.FromObject(0);
            GameDollars.FromObject(0);
            HonorPoints.FromObject(0);
            SkillPoints.FromObject(0);
            Gamertag.FromObject("");
            dateregistered.FromObject(DateTime.Now);
            lastlogindate.FromObject(new DateTime(1980, 1, 1));
            lastloginIP.FromObject("");
            lastgamedate.FromObject(new DateTime(1980, 1, 1));
            ReferralID.FromObject(0);
            IsFPSEnabled.FromObject(1);
            IsDeveloper.FromObject(0);
        }
    }
}
