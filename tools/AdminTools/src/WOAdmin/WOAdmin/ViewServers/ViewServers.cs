using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class ViewServers : Form, IAdminView
    {
        List<GameServerInfo> servers_ = new List<GameServerInfo>();
        FormViewServerInfo edit_ = null;

        public ViewServers()
        {
            InitializeComponent();

            servers_.Add(new GameServerInfo());
            lbLastUpdate.Text = "";

            panel1.Hide();
            edit_ = new FormViewServerInfo();
            edit_.TopLevel = false;
            edit_.FormBorderStyle = FormBorderStyle.None;
            edit_.Size = panel1.Size;
            edit_.Location = panel1.Location;
            edit_.Anchor = panel1.Anchor;
            edit_.DisableAll();
            edit_.Show();
            this.Controls.Add(edit_);
        }

        void IAdminView.OnViewActivated()
        {
        }

        void IAdminView.OnViewDeactivated()
        {
        }

        void PopulateFakeServerView()
        {
            listView1.SelectedIndices.Clear();
            listView1.Items.Clear();
            System.Threading.Thread.Sleep(1000);

            GameServerInfo gsi = servers_[0];

            ListViewItem lvi = new ListViewItem();
            lvi.Tag = gsi;
            gsi.ui_lvi = lvi;
            UpdateListView(gsi);

            listView1.Items.Add(lvi);
        }

        void UpdateListView(GameServerInfo gsi)
        {
            ListViewItem lvi = gsi.ui_lvi;

            lvi.UseItemStyleForSubItems = false;
            lvi.SubItems.Clear();
            lvi.Text = "LA Server 1";
            lvi.SubItems.Add("1");
            lvi.SubItems.Add("173.196.5.196");
            ListViewItem.ListViewSubItem si = lvi.SubItems.Add("HEALTHY");
            si.BackColor = Color.Green;
            si.ForeColor = Color.White;

            lvi.SubItems.Add("8/16");
            lvi.SubItems.Add("80/512");
        }

        private void btnRefresh_Click(object sender, EventArgs e)
        {
            PopulateFakeServerView();
            lbLastUpdate.Text = "Last Updated: " + DateTime.Now.ToString();
        }

        private void listView1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listView1.SelectedItems.Count == 0)
            {
                edit_.DisableAll();
                return;
            }

            GameServerInfo srv = listView1.SelectedItems[0].Tag as GameServerInfo;
            edit_.SetFromServerInfo(srv);
        }

    }
}
