using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class FormProcess : Form
    {
        public FormProcess()
        {
            this.TopLevel = false;
            this.Parent = Form1.THIS;

            InitializeComponent();
        }

        static FormProcess THIS = null;
        static public void Start(string msg)
        {
            if (THIS != null)
                return;
            THIS = new FormProcess();
            THIS.label1.Text = msg;
            THIS.Invalidate();
            THIS.Show();
            Application.DoEvents();
        }

        static public void Stop()
        {
            if (THIS == null)
                return;
            THIS.Dispose();
            THIS = null;
        }
    }
}
