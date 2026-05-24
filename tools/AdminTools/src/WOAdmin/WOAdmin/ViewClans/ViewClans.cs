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
    public partial class ViewClans : Form, IAdminView
    {
        FormEditClan edit_;
        SQLRec userContextRec_ = null;

        public ViewClans()
        {
            InitializeComponent();

            listView1.Dock = DockStyle.Fill;

            edit_ = new FormEditClan();
            edit_.TopLevel = false;
            edit_.Parent = splitContainer1.Panel2;
            edit_.FormBorderStyle = FormBorderStyle.None;
            edit_.Dock = DockStyle.Fill;
            edit_.DisableAll();
            edit_.AttachSaveButton(btnSave_Click);
            //edit_.AttachDeleteButton(btnDelete_Click);
            edit_.Show();

            Form1.clans_.AttachToListView(listView1);
            listView1.ListViewItemSorter = new ListViewColumnSorter();
            listView1.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
        }

        void IAdminView.OnViewActivated()
        {
            Form1.clans_.ReadFromDB();
            PopulateClanList();
        }

        void IAdminView.OnViewDeactivated()
        {
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.clans_.UpdateListView(er.sqlrec_);
        }

        private void btnDelete_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            int CustomerID = er.sqlrec_.RECORD_ID.AsInt();
            if (!er.ERDelete())
                return;
        }

        void PopulateClanList()
        {
            Form1.clans_.BeginListViewUpdate();

            foreach (var rec in Form1.clans_.data_.Values)
            {
                Form1.clans_.AddToListView(rec);
            }

            Form1.clans_.EndListViewUpdate();
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count == 0)
            {
                edit_.DisableAll();
                return;
            }

            var rec = listView1.SelectedItems[0].Tag as DBClanData;
            // reread current data
            Form1.sql.RefreshData(rec);
            Form1.clans_.UpdateListView(rec);
            edit_.SetFromRecord(rec);
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

            var ddd = new EditClanMembers(userContextRec_.RECORD_ID.AsInt());
            ddd.Show();
        }

        private void toolStripMenuItem2_Click(object sender, EventArgs e)
        {
            if (userContextRec_ == null)
                return;

            var ddd = new EditClanEvents(userContextRec_.RECORD_ID.AsInt(), true);
            ddd.Show();
        }

        private void tEMPResetAllFPSWeaponsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (userContextRec_ == null)
                return;

            var ddd = new EditClanEvents(userContextRec_.RECORD_ID.AsInt(), false);
            ddd.Show();
        }
    }
}
