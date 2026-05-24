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
    public partial class EditClanEvents : Form
    {
        int ClanID = 1000;
        bool GPFilter = false;
        List<DBClanEvent> events_ = new List<DBClanEvent>();

        public EditClanEvents(int in_ClanID, bool in_GPFilter)
        {
            InitializeComponent();

            ClanID = in_ClanID;
            lblUser.Text = ClanID.ToString();
            GPFilter = in_GPFilter;

            CreateListViewColumns();
            listView1.ListViewItemSorter = new ListViewColumnSorter();
            listView1.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);

            ReadClanData();
        }

        void ReadClanData()
        {
            events_.Clear();

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandText = String.Format(
                "select * from {0} where ClanID={1} order by ClanEventID asc", 
                DBClanEvent.GET_TABLE_NAME(), ClanID);

            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                var rec = new DBClanEvent();
                rec.Fill(reader);

                if (GPFilter)
                {
                    int eventType = rec.EventType.AsInt();
                    if (eventType != 10 && eventType != 11)
                        continue;
                }

                events_.Add(rec);
            }
            reader.Close();

            PopulateView();
        }

        void CreateListViewColumns()
        {
            ListView listView = listView1;
            listView.Columns.Clear();
            listView.Columns.Add("EventDate", 120);
            listView.Columns.Add("EventType", 100);
            listView.Columns.Add("Var1", 80);
            listView.Columns.Add("Var2", 80);
            listView.Columns.Add("Var3", 80);
            listView.Columns.Add("Var4", 80);
            listView.Columns.Add("Text1", 100);
            listView.Columns.Add("Text2", 100);
            listView.Columns.Add("Text3", 100);
        }

        void PopulateView()
        {
            listView1.BeginUpdate();
            listView1.SelectedIndices.Clear();
            listView1.Items.Clear();

            foreach (var rec in events_)
            {
                ListViewItem lvi = listView1.Items.Add("");
                lvi.Tag = rec;

                string eventType = DBClanEvent.EventTypeValues.GetNameByType(rec.EventType.ToString());

                lvi.SubItems.Clear();
                lvi.UseItemStyleForSubItems = false;
                lvi.Text = rec.EventDate.ToString();
                lvi.SubItems.Add(eventType);
                lvi.SubItems.Add(rec.Var1.ToString());
                lvi.SubItems.Add(rec.Var2.ToString());
                lvi.SubItems.Add(rec.Var3.ToString());
                lvi.SubItems.Add(rec.Var4.ToString());
                lvi.SubItems.Add(rec.Text1.ToString());
                lvi.SubItems.Add(rec.Text2.ToString());
                lvi.SubItems.Add(rec.Text3.ToString());
            }

            listView1.EndUpdate();
        }
    }
}
