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
    public partial class ViewUsers : Form, IAdminView
    {
        FormEditUser edit_;
        SQLRec userContextRec_ = null;

        public ViewUsers()
        {
            InitializeComponent();

            listView1.Dock = DockStyle.Fill;

            edit_ = new FormEditUser();
            edit_.TopLevel = false;
            edit_.Parent = splitContainer1.Panel2;
            edit_.FormBorderStyle = FormBorderStyle.None;
            edit_.Dock = DockStyle.Fill;
            edit_.DisableAll();
            edit_.AttachSaveButton(btnSave_Click);
            edit_.AttachDeleteButton(btnDelete_Click);
            edit_.Show();

            //ListViewItem lvi = listView1.Items.Add(new ListViewItem("avva"));
            //lvi.SubItems.Add("boo");

            Form1.users_.AttachToListView(listView1);
            listView1.ListViewItemSorter = new ListViewColumnSorter();
            listView1.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
        }

        void IAdminView.OnViewActivated()
        {
        }

        void IAdminView.OnViewDeactivated()
        {
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.users_.UpdateListView(er.sqlrec_);
        }

        private void btnDelete_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            int CustomerID = er.sqlrec_.RECORD_ID.AsInt();
            if (!er.ERDelete())
                return;

            // delete success, delete user from all other tables
            Form1.sql.Exec(string.Format("delete from AccountInfo where CustomerID={0}", CustomerID), false);
            Form1.sql.Exec(string.Format("delete from Profile_Loadouts where CustomerID={0}", CustomerID), false);
            Form1.sql.Exec(string.Format("delete from ProfileData where CustomerID={0}", CustomerID), false);
            Form1.sql.Exec(string.Format("delete from Stats where CustomerID={0}", CustomerID), false);
            Form1.sql.Exec(string.Format("delete from Logins where CustomerID={0}", CustomerID), false);
            Form1.sql.Exec(string.Format("delete from Inventory where CustomerID={0}", CustomerID), false);
            Form1.sql.Exec(string.Format("delete from SteamUserIDMap where CustomerID={0}", CustomerID), false);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            FormAddUser form = new FormAddUser();
            form.ShowDialog();
        }

        void ReadUserList(string name, string var)
        {
            if (var.Length < 3)
            {
                MessageBox.Show("too short query");
                return;
            }

            if (var.Substring(0, 1) == "=")
            {
                // search for exact match
                var = var.Trim();
                var = var.Remove(0, 1);

                Form1.users_.ReadUsersFromDB(name, "=", var);
            }
            else
            {
                // escape t-sql like regexp chars
                var = var.Trim();
                var = var.Replace("[", "[[]");
                var = var.Replace("_", "[_]");
                var = var.Replace("%", "[%]");

                Form1.users_.ReadUsersFromDB(name, "like", "%" + var + "%");
            }

            Form1.users_.BeginListViewUpdate();

            foreach (DBUser rec in Form1.users_.data_.Values)
            {
                Form1.users_.AddToListView(rec);
            }

            Form1.users_.EndListViewUpdate();
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count == 0)
            {
                edit_.DisableAll();
                return;
            }

            var rec = listView1.SelectedItems[0].Tag as DBUser;

            // reread current data
            Form1.sql.RefreshData(rec);
            Form1.users_.UpdateListView(rec);

            edit_.SetFromRecord(rec);
        }

        private void btnFilter1_Click(object sender, EventArgs e)
        {
            ReadUserList("AccountName", tbFilter1.Text);
        }

        private void btnFilter2_Click(object sender, EventArgs e)
        {
            ReadUserList("Gamertag", tbFilter2.Text);
        }

        private void btnFilter3_Click(object sender, EventArgs e)
        {
            ReadUserList("email", tbFilter3.Text);
        }

        private void btnFilter4_Click(object sender, EventArgs e)
        {
            ReadUserList("lastLoginIP", tbFilter4.Text);
        }

        private void btnFilter5_Click(object sender, EventArgs e)
        {
            try
            {
                Convert.ToInt32(tbFilter5.Text);
                ReadUserList("LoginID.CustomerID", "=" + tbFilter5.Text);
            }
            catch
            {
                MessageBox.Show("CustomerID is not a int");
            }
        }

        private void tbFilter1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                btnFilter1_Click(this, new EventArgs());
        }

        private void tbFilter2_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                btnFilter2_Click(this, new EventArgs());
        }

        private void tbFilter3_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                btnFilter3_Click(this, new EventArgs());
        }

        private void tbFilter4_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                btnFilter4_Click(this, new EventArgs());
        }

        private void tbFilter5_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                btnFilter5_Click(this, new EventArgs());
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
            if (userContextRec_ == null)
                return;

            EditUserItems form = new EditUserItems(userContextRec_ as DBUser);
            form.ShowDialog(this);
            userContextRec_ = null;
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            if (userContextRec_ == null)
                return;

            AddItemToUser form = new AddItemToUser(userContextRec_.RECORD_ID.AsInt());
            form.ShowDialog(this);
            userContextRec_ = null;
        }

        private void btnAddItem_Click(object sender, EventArgs e)
        {
            AddItemToUser form = new AddItemToUser(0);
            form.ShowDialog(this);
            userContextRec_ = null;
        }

        private void tEMPResetAllFPSWeaponsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (userContextRec_ == null)
                return;

            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "FN_MNT_ResetInventoryToFPS";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", userContextRec_.RECORD_ID.AsInt());

            SqlDataReader reader = null;
            Form1.sql.Select(sqcmd, out reader);
            reader.Close();

            MessageBox.Show("weapons resetted and all fps weapons added");
        }
    }
}
