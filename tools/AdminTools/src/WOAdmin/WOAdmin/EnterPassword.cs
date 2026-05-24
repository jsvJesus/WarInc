using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EnterPassword : Form
    {
        public EnterPassword()
        {
            InitializeComponent();
            comboBox1.SelectedIndex = Convert.ToInt32(Application.UserAppDataRegistry.GetValue("ServerId", 0));
            tbUser.Text = Convert.ToString(Application.UserAppDataRegistry.GetValue("User", ""));
        }

        private void EnterPassword_Shown(object sender, EventArgs e)
        {
            if (tbUser.Text.Length > 0)
                tbPasswd.Focus();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            Application.UserAppDataRegistry.SetValue("ServerId", comboBox1.SelectedIndex);
            Application.UserAppDataRegistry.SetValue("User", tbUser.Text);
            this.DialogResult = DialogResult.OK;
        }

        private void textBox1_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 13)
            {
                tbPasswd.Focus();
            }
        }

        private void textBox2_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (e.KeyChar == 13)
            {
                button1_Click(this, new EventArgs());
            }
        }

        private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            tbUser.Focus();
        }
    }
}
