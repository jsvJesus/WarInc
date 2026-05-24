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
    public partial class FormEditSKU : Form
    {
        Button btnSave_ = null;
        Button btnDelete_ = null;

        DBCommonItem sqlrec_ = null;

        public FormEditSKU()
        {
            InitializeComponent();
        }

        public void AttachSaveButton(/*System.EventHandler handler*/)
        {
            System.Diagnostics.Debug.Assert(btnSave_ == null);
            btnSave_ = new System.Windows.Forms.Button();
            btnSave_.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            btnSave_.BackColor = System.Drawing.SystemColors.ControlLight;
            btnSave_.Location = new System.Drawing.Point(this.Width - 100, 9);
            btnSave_.Name = "btnSave";
            btnSave_.Size = new System.Drawing.Size(75, 23);
            btnSave_.TabIndex = 15;
            btnSave_.Text = "Save";
            btnSave_.UseVisualStyleBackColor = false;
            btnSave_.Click += btnSaveSku_Click; // handler;
            this.Controls.Add(btnSave_);
        }

        public void AttachDeleteButton(/*System.EventHandler handler*/)
        {
            System.Diagnostics.Debug.Assert(btnDelete_ == null);
            btnDelete_ = new System.Windows.Forms.Button();
            btnDelete_.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            btnDelete_.BackColor = System.Drawing.SystemColors.ControlLight;
            btnDelete_.Location = new System.Drawing.Point(this.Width - 100, 40);
            btnDelete_.Name = "btnDelete";
            btnDelete_.Size = new System.Drawing.Size(75, 23);
            btnDelete_.TabIndex = 16;
            btnDelete_.Text = "Delete";
            btnDelete_.UseVisualStyleBackColor = false;
            btnDelete_.Click += btnDeleteSku_Click; // handler;
            this.Controls.Add(btnDelete_);
        }

        private void btnSaveSku_Click(object sender, EventArgs e)
        {
            if (sqlrec_ == null)
                return;

            try
            {
                sqlrec_.IsNew.v_int = cbIsNew.Checked ? 1 : 0;
                sqlrec_.Price1.v_int = Convert.ToInt32(textBox1.Text);
                sqlrec_.Price7.v_int = Convert.ToInt32(textBox7.Text);
                sqlrec_.Price30.v_int = Convert.ToInt32(textBox30.Text);
                sqlrec_.PriceP.v_int = Convert.ToInt32(textBoxP.Text);
                sqlrec_.GPrice1.v_int = Convert.ToInt32(textBox1G.Text);
                sqlrec_.GPrice7.v_int = Convert.ToInt32(textBox7G.Text);
                sqlrec_.GPrice30.v_int = Convert.ToInt32(textBox30G.Text);
                sqlrec_.GPriceP.v_int = Convert.ToInt32(textBoxPG.Text);
            }
            catch(Exception)
            {
                MessageBox.Show("price is not a number");
                return;
            }

            Form1.sql.Update(sqlrec_);
            SetFromPrice(sqlrec_);

            // some weird hacks to update listviews...
            if ((sqlrec_ as DBWeapon) != null)
                Form1.weapons_.UpdateListView(sqlrec_);
            else if ((sqlrec_ as DBGear) != null)
                Form1.gears_.UpdateListView(sqlrec_);
            else if ((sqlrec_ as DBItem) != null)
                Form1.items_.UpdateListView(sqlrec_);
            else if ((sqlrec_ as DBPackage) != null)
                Form1.packages_.UpdateListView(sqlrec_);
        }

        void SetPricesToZero()
        {
            cbIsNew.Checked = false;
            textBox1.Text = "0";
            textBox7.Text = "0";
            textBox30.Text = "0";
            textBoxP.Text = "0";
            textBox1G.Text = "0";
            textBox7G.Text = "0";
            textBox30G.Text = "0";
            textBoxPG.Text = "0";
        }

        private void btnDeleteSku_Click(object sender, EventArgs _e)
        {
            if (sqlrec_ == null)
                return;

            SetPricesToZero();

            btnSaveSku_Click(null, _e);
        }

        public void DisableAll()
        {
            sqlrec_ = null;
            SetPricesToZero();
        }

        public void SetFromPrice(DBCommonItem in_rec)
        {
            sqlrec_ = in_rec;
            if (btnSave_ != null)
                btnSave_.Enabled = true;

            lbItemName.Text = sqlrec_.RECORD_ID.ToString();

            cbIsNew.Checked = sqlrec_.IsNew.AsInt() > 0;
            textBox1.Text = sqlrec_.Price1.ToString();
            textBox7.Text = sqlrec_.Price7.ToString();
            textBox30.Text = sqlrec_.Price30.ToString();
            textBoxP.Text = sqlrec_.PriceP.ToString();
            textBox1G.Text = sqlrec_.GPrice1.ToString();
            textBox7G.Text = sqlrec_.GPrice7.ToString();
            textBox30G.Text = sqlrec_.GPrice30.ToString();
            textBoxPG.Text = sqlrec_.GPriceP.ToString();
        }
    }
}
