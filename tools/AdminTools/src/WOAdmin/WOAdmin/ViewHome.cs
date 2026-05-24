using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class ViewHome : Form, IAdminView
    {
        public ViewHome()
        {
            InitializeComponent();

            //webBrowser1.Navigate(@"https://gameid.arktosentertainment.com/expandablemenu.php");
        }

        void IAdminView.OnViewActivated()
        {
        }

        void IAdminView.OnViewDeactivated()
        {
        }

    }
}
