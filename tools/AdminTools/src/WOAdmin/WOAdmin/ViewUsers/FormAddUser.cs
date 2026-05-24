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
    public partial class FormAddUser : Form
    {
        List<int> referrerIds = new List<int>();

        public FormAddUser()
        {
            InitializeComponent();

            //CreateBatchUsers();

            // set varchar max lenghts
            tbUserName.MaxLength = 16;
            tbPassword.MaxLength = 16;
            tbEmail.MaxLength = 64;
        }

        bool ValidateInput()
        {
            if (tbUserName.Text.Length < 1)
                return false;
            if (tbPassword.Text.Length < 1)
                return false;
            if (tbEmail.Text.Length < 1)
                return false;
            return true;
        }

        /*
        void CreateBatchUser(string user, int min, int max)
        {
            for(int i=min; i<=max; i++)
            {
                SqlDataReader reader = null;

                try
                {
                    SqlCommand sqcmd = new SqlCommand();

                    string name = user + i.ToString();

                    sqcmd.CommandType = CommandType.StoredProcedure;
                    sqcmd.CommandText = "ECLIPSE_CREATEACCOUNT";
                    sqcmd.Parameters.AddWithValue("@in_IP", "0.0.0.0");
                    sqcmd.Parameters.AddWithValue("@in_Username", name);
                    sqcmd.Parameters.AddWithValue("@in_Password", "12345");
                    sqcmd.Parameters.AddWithValue("@in_Email", name + "@test.com");
                    sqcmd.Parameters.AddWithValue("@in_Reg_SID", "");
                    sqcmd.Parameters.AddWithValue("@in_ReferralID", 1);

                    Form1.sql.Select(sqcmd, out reader);
                    reader.Read();

                    string ResultCode = reader["ResultCode"].ToString();

                    if (ResultCode != "0")
                    {
                        string ResultMsg = reader["ResultMsg"].ToString();
                        MessageBox.Show("Failed to add user\n" + ResultMsg, "User adding");
                    }
                }
                catch (Exception e)
                {
                    MessageBox.Show("Failed to add user\n" + e.Message, "User adding");
                }
                finally
                {
                    if (reader != null) reader.Close();
                }
            }
        }

        void CreateBatchUsers()
        {
            CreateBatchUser("AGames", 1, 8);
            CreateBatchUser("BPoint", 1, 8);
            CreateBatchUser("CYou", 1, 8);
            CreateBatchUser("GalNet", 1, 8);
            CreateBatchUser("GForge", 1, 8);
            CreateBatchUser("Msft", 1, 8);
            CreateBatchUser("NeWiz", 1, 8);
            CreateBatchUser("Nxon", 1, 8);
            CreateBatchUser("Snda", 1, 8);
            CreateBatchUser("Tcent", 1, 12);
            CreateBatchUser("OWA", 1, 20);
        }
        */

        SqlCommand CreateSQLRequest()
        {
            SqlCommand sqcmd = new SqlCommand();

            int refId = 1288871140; //REFERID_Arktos

            sqcmd.CommandType = CommandType.StoredProcedure;
            sqcmd.CommandText = "ECLIPSE_CREATEACCOUNT";
            sqcmd.Parameters.AddWithValue("@in_IP", "0.0.0.0");
            sqcmd.Parameters.AddWithValue("@in_Username", tbUserName.Text);
            sqcmd.Parameters.AddWithValue("@in_Password", tbPassword.Text);
            sqcmd.Parameters.AddWithValue("@in_Email", tbEmail.Text);
            sqcmd.Parameters.AddWithValue("@in_Reg_SID", "");
            sqcmd.Parameters.AddWithValue("@in_ReferralID", refId);

            return sqcmd;
        }

        bool DoCreateUser()
        {
            SqlDataReader reader = null;
            try
            {
                SqlCommand sqcmd = CreateSQLRequest();

                Form1.sql.Select(sqcmd, out reader);
                reader.Read();

                string ResultCode = reader["ResultCode"].ToString();
                if (ResultCode != "0")
                {
                    string ResultMsg = reader["ResultMsg"].ToString();
                    MessageBox.Show("Failed to add user\n" + ResultMsg, "User adding");
                    return false;
                }

                MessageBox.Show("User created successfully, please reread user list", "User adding");
                return true;
            }
            catch (Exception e)
            {
                MessageBox.Show("Error: " + e.Message, "User adding");
                return false;
            }
            finally
            {
                if (reader != null) reader.Close();
            }
        }

        private void btnCreate_Click(object sender, EventArgs e)
        {
            if (!ValidateInput())
            {
                MessageBox.Show("Please fill all needed fields", "");
                return;
            }

            if (!DoCreateUser())
                return;

            Close();
        }
    }
}
