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
    public partial class ViewSecurityLog : Form, IAdminView
    {
        List<DBSecurityLogRO> transactions_ = new List<DBSecurityLogRO>();
        Dictionary<int, String> eventIdToName = new Dictionary<int, String>();

        public void PopulateEvenIdNames()
        {
            eventIdToName.Add(100, "Login: bad username");
            eventIdToName.Add(101, "Login: bad password");

            eventIdToName.Add(110, "BuyItem: bad itemid");
            eventIdToName.Add(111, "BuyItem: no itemid");
            eventIdToName.Add(112, "BuyItem: bad buydays");
            eventIdToName.Add(113, "BuyItem: buying not listed item");
            eventIdToName.Add(114, "BuyItem: not enough money");

            eventIdToName.Add(120, "Skill: bad skill id");
            eventIdToName.Add(121, "Skill: not enough sp");
            eventIdToName.Add(130, "Skill: bad ability id");
            eventIdToName.Add(131, "Skill: not enough gp");

            eventIdToName.Add(141, "Slot: not enough gp");

            eventIdToName.Add(150, "Loadout: bad slot id");
            eventIdToName.Add(151, "Loadout: bad item id");

            eventIdToName.Add(160, "Welcome: bad input");
            eventIdToName.Add(161, "Welcome: bad AccountStatus");

            eventIdToName.Add(170, "!!!UpdateProfile: bad user id");
            eventIdToName.Add(171, "!!!UpdateProfile: bad skey2");

            eventIdToName.Add(300, "!!! bad itemId in ECLIPSE_PROCESSTRANSACTION");
            eventIdToName.Add(301, "!!! bad serverId in WO_GetCreateGameKey2");

            eventIdToName.Add(400, "in game cheat");
        }

        public string GetEventName(int eventId)
        {
            string name;
            if (eventIdToName.TryGetValue(eventId, out name))
                return name;
            return eventId.ToString();
        }

        public ViewSecurityLog()
        {
            InitializeComponent();

            PopulateEvenIdNames();

            ListView listView = listViewR;
            listView.Columns.Clear();
            listView.Columns.Add("EventID", 150);
            listView.Columns.Add("Date", 120);
            listView.Columns.Add("IP", 100);
            listView.Columns.Add("CustomerID", 100);
            listView.Columns.Add("Data", 300);

            listViewR.Size = new Size(listViewR.Size.Width, 300);
        }

        void IAdminView.OnViewActivated()
        {
        }

        void IAdminView.OnViewDeactivated()
        {
        }


        SqlCommand CreateSQLRequest()
        {
            string table = DBSecurityLogRO.GET_TABLE_NAME();

            DateTime dateFrom = dtpickFrom.Value;
            DateTime dateTo = dtpickTo.Value;

            // build sql command and where string
            SqlCommand sqcmd = new SqlCommand();

            string where = "WHERE ";

            // date time range
            if(true)
            {
                where += "(Date BETWEEN @dtFrom AND @dtTo)";
                sqcmd.Parameters.AddWithValue("@dtFrom", dateFrom);
                sqcmd.Parameters.AddWithValue("@dtTo", dateTo);
            }

            sqcmd.CommandText = String.Format("select * from {0} {1}", table, where);

            return sqcmd;
        }

        void ReadTransactionDB()
        {
            transactions_.Clear();

            SqlCommand sqcmd = CreateSQLRequest();
            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                DBSecurityLogRO rec = new DBSecurityLogRO();
                rec.Fill(reader);

                transactions_.Add(rec);
            }
            reader.Close();
        }

        void PopulateView()
        {
            listViewR.BeginUpdate();
            listViewR.SelectedIndices.Clear();
            listViewR.Items.Clear();

            foreach (DBSecurityLogRO rec in transactions_)
            {
                string userName = Form1.GetUserName(rec.CustomerID.AsInt());
                string eventName = GetEventName(rec.EventID.v_int);

                ListViewItem lvi = listViewR.Items.Add(eventName);
                lvi.SubItems.Add(rec.Date.ToString());
                lvi.SubItems.Add(rec.IP.ToString());
                lvi.SubItems.Add(userName);
                lvi.SubItems.Add(rec.EventData.ToString());
                lvi.Tag = rec;
            }

            listViewR.EndUpdate();
        }

        private void btnUpdate_Click(object sender, EventArgs e)
        {
            ReadTransactionDB();
            PopulateView();
        }
    }
}
