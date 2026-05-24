using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public class WOUsers : UIDBListView<DBUser>
    {
        public WOUsers()
        {
        }

        override protected string GET_TABLE_NAME()
        {
            return DBUser.GET_TABLE_NAME();
        }

        public void ReadUsersFromDB(string where_name, string comp_func, string where_var)
        {
            data_.Clear();

            string query = "select ";
            query += "LoginID.*,";
            query += "AccountInfo.email, AccountInfo.admin_note,";
            query += "Stats.Kills,";
            query += "Stats.Deaths,";
            query += "Stats.ShotsFired,";
            query += "Stats.ShotsHits,";
            query += "Stats.Headshots,";
            query += "Stats.Wins,";
            query += "Stats.Losses,";
            query += "Stats.CaptureNeutralPoints,";
            query += "Stats.CaptureEnemyPoints,";
            query += "Stats.TimePlayed ";

            query += "from loginid ";
            query += "join AccountInfo on AccountInfo.CustomerID=LoginID.CustomerID ";
            query += "join Stats on Stats.CustomerID=LoginID.CustomerID ";
            query += string.Format("where {0} {1} @where", where_name, comp_func);

            System.Data.SqlClient.SqlCommand sqcmd = new System.Data.SqlClient.SqlCommand();
            sqcmd.CommandText = query;
            sqcmd.Parameters.AddWithValue("@where", where_var);

            System.Data.SqlClient.SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                DBUser rec1 = new DBUser();
                rec1.Fill(reader);
                data_.Add(rec1.RECORD_ID.v_int, rec1);

                rec1.refl_AccountInfo = new DBAccountInfo();
                rec1.refl_AccountInfo.Fill(reader);
                rec1.refl_Stats = new DBStatsRO();
                rec1.refl_Stats.Fill(reader);
            }
            reader.Close();
        }

        override protected void CreateListViewColumns()
        {
            listView.Columns.Clear();
            listView.Columns.Add("Name", 120);
            listView.Columns.Add("Status", 80);
            listView.Columns.Add("Gamertag", 100);
            listView.Columns.Add("GamePoints", 80);
            listView.Columns.Add("GameDollars", 80);
            listView.Columns.Add("SkillPoints", 80);
            listView.Columns.Add("HonorPoints", 80);
            listView.Columns.Add("email", 120);
            listView.Columns.Add("LastLogin", 150);
            listView.Columns.Add("LastIP", 120);
            listView.Columns.Add("LastGame", 150);
            listView.Columns.Add("Referrer", 100);
            listView.Columns.Add("admin_note", 150);
        }

        override public void UpdateListView(SQLRec in_rec)
        {
            ListViewItem lvi = in_rec.ui_lvi;
            System.Diagnostics.Debug.Assert(lvi != null);
            DBUser rec = in_rec as DBUser;
            System.Diagnostics.Debug.Assert(rec != null);

            lvi.Tag = rec;
            lvi.SubItems.Clear();
            lvi.Text = rec.AccountName.ToString();
            lvi.SubItems.Add(DBUser.AccountStatusValues.GetNameByType(rec.AccountStatus.ToString()));
            lvi.SubItems.Add(rec.Gamertag.ToString());
            lvi.SubItems.Add(rec.GamePoints.ToString());
            lvi.SubItems.Add(rec.GameDollars.ToString());
            lvi.SubItems.Add(rec.SkillPoints.ToString());
            lvi.SubItems.Add(rec.HonorPoints.ToString());
            lvi.SubItems.Add(rec.refl_AccountInfo == null ? "<no accinfo>" : rec.refl_AccountInfo.email.ToString());
            lvi.SubItems.Add(rec.lastlogindate.ToString());
            lvi.SubItems.Add(rec.lastloginIP.ToString());
            lvi.SubItems.Add(rec.lastgamedate.ToString());
            lvi.SubItems.Add(rec.ReferralID.ToString());
            lvi.SubItems.Add(rec.refl_AccountInfo == null ? "<no accinfo>" : rec.refl_AccountInfo.admin_note.ToString());
        }
    }
}
