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
    public partial class EditUserItems : Form
    {
        int CustomerID = 1000;
        List<DBInventory> inventory_ = new List<DBInventory>();
        SQLRec userContextRec_ = null;

        public EditUserItems(DBUser in_rec)
        {
            InitializeComponent();

            CustomerID = in_rec.RECORD_ID.AsInt();
            lblUser.Text = CustomerID.ToString();

            CreateListViewColumns();

            listView1.ListViewItemSorter = new ListViewColumnSorter();
            listView1.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);

            ReadUserItems();
        }

        void CreateListViewColumns()
        {
            ListView listView = listView1;
            listView.Columns.Clear();
            listView.Columns.Add("ItemID", 120);
            listView.Columns.Add("ExpDate", 150);
            listView.Columns.Add("ItemName", 250);
            listView.Columns.Add("ItemDesc", 1024);
        }

        void ReadUserItems()
        {
            inventory_.Clear();

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandText = String.Format("select * from {0} where CustomerID={1} order by ItemID asc", DBInventory.GET_TABLE_NAME(), CustomerID);

            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                DBInventory rec = new DBInventory();
                rec.Fill(reader);

                inventory_.Add(rec);
            }
            reader.Close();

            PopulateView();
        }

        void PopulateView()
        {
            listView1.BeginUpdate();
            listView1.SelectedIndices.Clear();
            listView1.Items.Clear();

            foreach (DBInventory rec in inventory_)
            {
                ListViewItem lvi = listView1.Items.Add("");
                lvi.Tag = rec;

                string itemName = "";
                string itemDesc = "";
                Form1.THIS.GetItemInfo(rec.ItemID.AsInt(), ref itemName, ref itemDesc);

                lvi.SubItems.Clear();
                lvi.UseItemStyleForSubItems = false;
                lvi.Text = rec.ItemID.ToString();
                ListViewItem.ListViewSubItem leased = lvi.SubItems.Add(rec.LeasedUntil.ToString());
                lvi.SubItems.Add(itemName);
                lvi.SubItems.Add(itemDesc);

                if(rec.LeasedUntil.v_time < DateTime.Now)
                    leased.BackColor = System.Drawing.Color.Salmon; // FromArgb(180, 255, 255);
            }

            listView1.EndUpdate();
        }

        private void listView1_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                ListViewHitTestInfo hitTestInfo = listView1.HitTest(e.X, e.Y);
                if (hitTestInfo.Item != null)
                {
                    userContextRec_ = hitTestInfo.Item.Tag as SQLRec;

                    //show the context menu strip
                    contextMenuStrip1.Show(listView1, e.X, e.Y);
                }
            }
        }

        private void toolStripMenuItem1_Click(object sender, EventArgs e)
        {
            DBInventory inv = userContextRec_ as DBInventory;

            string cmd = string.Format("delete from Inventory where CustomerID={0} and ItemID={1}",
                inv.CustomerID.AsInt(),
                inv.ItemID.AsInt());
            Form1.sql.Exec(cmd, true);

            /*
            cmd = string.Format("delete from Inventory_FPS where CustomerID={0} and WeaponID={1}",
                inv.CustomerID.AsInt(),
                inv.ItemID.AsInt());
            Form1.sql.Exec(cmd, true);
             * */


            ReadUserItems();
        }
    }
}
