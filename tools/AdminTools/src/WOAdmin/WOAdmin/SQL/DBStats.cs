using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
    CustomerID	int	Unchecked
    Kills	int	Unchecked
    Deaths	int	Unchecked
    ShotsFired	int	Unchecked
    ShotsHits	int	Unchecked
    Headshots	int	Unchecked
    AssistKills	int	Unchecked
    Wins	int	Unchecked
    Losses	int	Unchecked
    CaptureNeutralPoints	int	Unchecked
    CaptureEnemyPoints	int	Unchecked
    TimePlayed	int	Unchecked
    CheatAttempts	int	Unchecked
    * * */

    public class DBStatsRO : SQLRec
    {
        public SQLField Kills = new SQLField("Kills");
        public SQLField Deaths = new SQLField("Deaths");
        public SQLField ShotsFired = new SQLField("ShotsFired");
        public SQLField ShotsHits = new SQLField("ShotsHits");
        public SQLField Headshots = new SQLField("Headshots");
        public SQLField Wins = new SQLField("Wins");
        public SQLField Losses = new SQLField("Losses");
        public SQLField CaptureNeutralPoints = new SQLField("CaptureNeutralPoints");
        public SQLField CaptureEnemyPoints = new SQLField("CaptureEnemyPoints");
        public SQLField TimePlayed = new SQLField("TimePlayed");

        public static string GET_TABLE_NAME()
        {
            return "Stats";
        }

        public DBStatsRO()
        {
            TABLE_ID = GET_TABLE_NAME();
            //RECORD_ID = new SQLField("CustomerID");

            RegisterField(Kills);
            RegisterField(Deaths);
            RegisterField(ShotsFired);
            RegisterField(ShotsHits);
            RegisterField(Headshots);
            RegisterField(Wins);
            RegisterField(Losses);
            RegisterField(CaptureNeutralPoints);
            RegisterField(CaptureEnemyPoints);
            RegisterField(TimePlayed);
        }
    }
}
