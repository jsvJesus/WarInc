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
    public partial class ViewTransactions : Form, IAdminView
    {
        List<DBTransactionRO> transactions_ = new List<DBTransactionRO>();

        public ViewTransactions()
        {
            InitializeComponent();

            foreach (EditField_Combo.SPair pair in DBTransactionRO.TransactionTypeValues)
            {
                cbTransType.Items.Add(pair.name);
            }
            cbTransType.SelectedIndex = 1;

            listViewR.Hide();
            listViewG.Hide();
            listViewR.Size = new Size(listViewR.Size.Width, 300);
            listViewG.Location = listViewR.Location;
            listViewG.Size = listViewR.Size;

            CreateListViewColumns_Money(listViewR);

            listViewR.ListViewItemSorter = new ListViewColumnSorter();
            listViewR.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
            listViewG.ListViewItemSorter = new ListViewColumnSorter();
            listViewG.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
        }

        void CreateListViewColumns_Money(ListView listView)
        {
            listView.Columns.Clear();
            listView.Columns.Add("CustomerID", 80);
            listView.Columns.Add("TransactionID", 300);
            listView.Columns.Add("DateTime", 120);
            listView.Columns.Add("Amount", 60);
            listView.Columns.Add("ResponseCode", 250);
            listView.Columns.Add("ItemID", 80);
            listView.Columns.Add("Approval", 150);
        }

        void IAdminView.OnViewActivated()
        {
        }

        void IAdminView.OnViewDeactivated()
        {
        }

        public void CopyListViewToClipboard(ListView lv)
        {
            StringBuilder buffer = new StringBuilder();

            foreach (ListViewItem item in lv.SelectedItems)
            {
                for (int j = 0; j < lv.Columns.Count; j++)
                {
                    buffer.Append(item.SubItems[j].Text);
                    buffer.Append("\t");
                }
                buffer.Append("\n");
            }

            Clipboard.SetText(buffer.ToString());
        }

        SqlCommand CreateSQLRequest(int CustomerID)
        {
            string table = DBTransactionRO.GET_TABLE_NAME();

            // create filter variables
            int transactionType = Convert.ToInt32(
                DBTransactionRO.TransactionTypeValues[cbTransType.SelectedIndex].type);

            string ItemID = tbItemID.Text;

            DateTime dateFrom = dtpickFrom.Value;
            DateTime dateTo = dtpickTo.Value;

            // build sql command and where string
            SqlCommand sqcmd = new SqlCommand();

            string where = "WHERE 1=1";
            
            if(transactionType > 0) 
            {
                where += " AND (TransactionType = @TransactionType)";
                sqcmd.Parameters.AddWithValue("@TransactionType", transactionType);
            }

            if (CustomerID > 0)
            {
                where += " AND (CustomerID = @CustomerID)";
                sqcmd.Parameters.AddWithValue("@CustomerID", CustomerID);
            }

            // date time range
            if(true)
            {
                where += " AND (DateTime BETWEEN @dtFrom AND @dtTo)";
                sqcmd.Parameters.AddWithValue("@dtFrom", dateFrom);
                sqcmd.Parameters.AddWithValue("@dtTo", dateTo);
            }

            if (ItemID.Length > 0)
            {
                where += " AND (ItemID LIKE @ItemID)";
                sqcmd.Parameters.AddWithValue("@ItemID", ItemID);
            }

            sqcmd.CommandText = String.Format("select * from {0} {1} order by DateTime desc", table, where);

            return sqcmd;
        }

        void ReadTransactionDB()
        {
            int CustomerID = 0;
            try
            {
                CustomerID = Convert.ToInt32(tbCustomerID.Text);
            }
            catch { }

            if (CustomerID == 0 && cbTransType.SelectedIndex != 1)
            {
                MessageBox.Show("enter CustomerID");
                return;
            }


            transactions_.Clear();

            SqlCommand sqcmd = CreateSQLRequest(CustomerID);
            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                DBTransactionRO rec = new DBTransactionRO();
                rec.Fill(reader);

                transactions_.Add(rec);
            }
            reader.Close();
        }

        void PopulateRealMoneyView()
        {
            listViewR.BeginUpdate();
            listViewR.SelectedIndices.Clear();
            listViewR.Items.Clear();

            foreach (DBTransactionRO rec in transactions_)
            {
                string userName = Form1.GetUserName(rec.CustomerID.AsInt());

                string itemName = rec.ItemID.ToString();
                string itemDesc = "";
                try
                {
                    // itemID in transaction base is a string
                    int id = Convert.ToInt32(rec.ItemID.ToString());
                    if(id > 0)
                        Form1.THIS.GetItemInfo(id, ref itemName, ref itemDesc);
                }
                catch { }

                ListViewItem lvi = listViewR.Items.Add(new ListViewItem(userName));
                lvi.SubItems.Add(rec.TransactionID.ToString());
                lvi.SubItems.Add(rec.DateTime.ToString());
                lvi.SubItems.Add(rec.Amount.ToString());
                lvi.SubItems.Add(rec.ResponseCode.ToString());
                lvi.SubItems.Add(itemName);
                lvi.SubItems.Add(rec.AprovalCode.ToString());
                lvi.Tag = rec;
            }

            listViewR.EndUpdate();
        }

        void PopulateGameMoneyView()
        {
            listViewG.BeginUpdate();
            listViewG.SelectedIndices.Clear();
            listViewG.Items.Clear();

            foreach (DBTransactionRO rec in transactions_)
            {
                string userName = Form1.GetUserName(rec.CustomerID.AsInt());

                string itemName = "";
                string itemDesc = "";
                try
                {
                    // itemID in transaction base is a string
                    int id = Convert.ToInt32(rec.ItemID.ToString());
                    Form1.THIS.GetItemInfo(id, ref itemName, ref itemDesc);
                }
                catch { }

                ListViewItem lvi = listViewG.Items.Add(new ListViewItem(userName));
                lvi.SubItems.Add(DBTransactionRO.TransactionTypeValues.GetNameByType(
                    rec.TransactionType.ToString()));
                lvi.SubItems.Add(rec.DateTime.ToString());
                lvi.SubItems.Add(rec.Amount.ToString());
                lvi.SubItems.Add(rec.ItemID.ToString());
                lvi.SubItems.Add(itemName + " - " + itemDesc);
                lvi.Tag = rec;
            }

            listViewG.EndUpdate();
        }

        private void btnUpdate_Click(object sender, EventArgs e)
        {
            ReadTransactionDB();
            if (cbTransType.SelectedIndex < 2)
            {
                listViewG.Hide();
                listViewR.Show();
                PopulateRealMoneyView();
            }
            else
            {
                listViewG.Show();
                listViewR.Hide();
                PopulateGameMoneyView();
            }
        }

        private void listViewR_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.C && e.Control)
            {
                CopyListViewToClipboard(sender as ListView);
                MessageBox.Show("Selected row is copied to clipboard");
            }
        }

        private void listViewR_SelectedIndexChanged(object sender, EventArgs e)
        {
            ListView lv = sender as ListView;
            if (lv.SelectedItems.Count == 0)
                return;

            textBoxCustomerID.Text = lv.SelectedItems[0].SubItems[0].Text;
        }

        private void cbTransType_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cb = sender as ComboBox;
            if(cb.SelectedIndex == 1)
                dtpickFrom.Value = DateTime.Now.Subtract(new TimeSpan(24 * 30, 0, 0));
            else
                dtpickFrom.Value = new DateTime(2010, 1, 1);
        }
    }
}
