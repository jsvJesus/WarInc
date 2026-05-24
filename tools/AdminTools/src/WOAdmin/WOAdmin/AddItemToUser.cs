using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class AddItemToUser : Form
    {
        public AddItemToUser(int in_CustomerID)
        {
            InitializeComponent();

            if (in_CustomerID > 0)
                tbCustomer.Text = in_CustomerID.ToString();
        }

        private void AddItemToUser_Shown(object sender, EventArgs e)
        {
            if(tbCustomer.Text.Length > 0)
                tbId1.Focus();
        }

        void AddItem(ref string msg, int CustomerID, TextBox tbId, TextBox tbExp)
        {
            if (tbId.Text.Length == 0 && tbExp.Text.Length == 0)
                return;

            // parse
            int ItemID = 0;
            int ExpDays = 0;
            try
            {
                ItemID = Convert.ToInt32(tbId.Text);
                ExpDays = Convert.ToInt32(tbExp.Text);
            }
            catch
            {
                msg += string.Format("  Invalid ItemId or ExpDays\n");
                return;
            }

            // get item description
            string name = "";
            string desc = "";
            int cat = 0;
            if (!Form1.THIS.GetItemInfo(ItemID, ref name, ref desc, ref cat))
            {
                msg += string.Format("  ItemId {0} not found\n", ItemID);
                return;
            }

            // add 
            System.Data.SqlClient.SqlCommand sqcmd = new System.Data.SqlClient.SqlCommand();
            sqcmd.CommandText = "exec FN_AddItemToUser @in_CustomerID, @in_ItemID, @in_ExpDays";
            sqcmd.Parameters.AddWithValue("@in_CustomerID", CustomerID);
            sqcmd.Parameters.AddWithValue("@in_ItemID", ItemID);
            sqcmd.Parameters.AddWithValue("@in_ExpDays", ExpDays);

            System.Data.SqlClient.SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            reader.Close();

            msg += string.Format("  {0}:{1} added for {2} days\n", ItemID, name, ExpDays);
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int CustomerID = 0;
            if (tbCustomer.Text.Length > 0)
            {
                try
                {
                    CustomerID = Convert.ToInt32(tbCustomer.Text);
                }
                catch
                {
                    MessageBox.Show("bad CustomerID");
                    return;
                }
            }
            else if (tbGamertag.Text.Length > 0)
            {
                System.Data.SqlClient.SqlCommand sqcmd = new System.Data.SqlClient.SqlCommand();
                sqcmd.CommandText = "select CustomerID from LoginID where Gamertag=@Gamertag";
                sqcmd.Parameters.AddWithValue("@Gamertag", tbGamertag.Text);

                System.Data.SqlClient.SqlDataReader reader;
                Form1.sql.Select(sqcmd, out reader);
                if (!reader.Read())
                {
                    MessageBox.Show("gamertag not found");
                    return;
                }
                CustomerID = Convert.ToInt32(reader["CustomerID"]);
                reader.Close();
            }
            else
            {
                MessageBox.Show("need customerid or gamertag");
                return;
            }

            string msg = string.Format("Adding items to {0}\n", CustomerID);
            AddItem(ref msg, CustomerID, tbId1, tbExp1);
            AddItem(ref msg, CustomerID, tbId2, tbExp2);
            AddItem(ref msg, CustomerID, tbId3, tbExp3);
            AddItem(ref msg, CustomerID, tbId4, tbExp4);
            AddItem(ref msg, CustomerID, tbId5, tbExp5);
            MessageBox.Show(msg);
            return;
        }
    }
}
