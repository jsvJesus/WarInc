using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormViewServerInfo : Form
    {
        public FormViewServerInfo()
        {
            InitializeComponent();
        }

        public void DisableAll()
        {
            Hide();
        }

        public void SetFromServerInfo(GameServerInfo srv)
        {
            Show();
        }
    }
}
