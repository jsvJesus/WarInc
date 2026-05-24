using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    public partial class EditClanMembers : Form
    {
        int ClanID = 1000;
        List<DBUser> members_ = new List<DBUser>();

        public EditClanMembers(int in_ClanID)
        {
            InitializeComponent();

            ClanID = in_ClanID;
            lblUser.Text = ClanID.ToString();

            CreateListViewColumns();

            listView1.ListViewItemSorter = new ListViewColumnSorter();
            listView1.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);

            ReadClanData();
        }

        void ReadClanData()
        {
            members_.Clear();

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandText = String.Format(
                "select * from LoginID where ClanID={0} order by ClanRank asc", 
                ClanID);

            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                var rec = new DBUser();
                rec.Fill(reader);
                members_.Add(rec);
            }
            reader.Close();

            PopulateView();
        }

        void CreateListViewColumns()
        {
            ListView listView = listView1;
            listView.Columns.Clear();
            listView.Columns.Add("CustomerID", 120);
            listView.Columns.Add("ClanRank", 100);
            listView.Columns.Add("Gamertag", 150);
            listView.Columns.Add("ContrbGP", 100);
            listView.Columns.Add("ContrbXP", 100);
        }

        void PopulateView()
        {
            listView1.BeginUpdate();
            listView1.SelectedIndices.Clear();
            listView1.Items.Clear();

            foreach (var rec in members_)
            {
                ListViewItem lvi = listView1.Items.Add("");
                lvi.Tag = rec;

                lvi.SubItems.Clear();
                lvi.UseItemStyleForSubItems = false;
                lvi.Text = rec.RECORD_ID.ToString();
                lvi.SubItems.Add(rec.ClanRank.ToString());
                lvi.SubItems.Add(rec.Gamertag.ToString());
                lvi.SubItems.Add(rec.ClanContributedGP.ToString());
                lvi.SubItems.Add(rec.ClanContributedXP.ToString());
            }

            listView1.EndUpdate();
        }
    }
}
