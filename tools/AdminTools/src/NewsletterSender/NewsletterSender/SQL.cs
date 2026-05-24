using System;
using System.Collections.Generic;
using System.Text;
using System.Data.SqlClient;
using System.Diagnostics;
using System.Windows.Forms;

namespace NewsletterSender
{
    public class SQLBase
    {
        string server = "192.95.7.127,4685";
        string user = "sa";
        public string pass = "SqL2090SenhaPika@#";
        string workdb = "WorldZ_DB"; // gameid_v1

        SqlConnection conn_ = null;

        public SQLBase()
        {
        }

        ~SQLBase()
        {
            if (conn_ == null)
                return;

            try
            {
                conn_.Close();
            }
            catch { }
            conn_ = null;
        }

        bool Crap(Exception e)
        {
            MessageBox.Show(e.Message, "SQL error");
            return false;
        }

        public bool Connect()
        {
            if (pass.Length == 0)
                throw new ArgumentException("no password in sql");

            string str = String.Format(
                "user id={0};" +
                "password={1};server={2};" +
                "Trusted_Connection=false;" +
                "database={3};" +
                "connection timeout=30",
                user,
                pass,
                server,
                workdb
                );

            try
            {
                SqlConnection c = new SqlConnection(str);
                c.Open();

                conn_ = c;
            }
            catch (Exception e)
            {
                Crap(e);
                return false;
            }

            return true;
        }

        void DumpSqlCommand(SqlCommand sqcmd)
        {
            Debug.WriteLine("CMD: " + sqcmd.CommandText);

            foreach (SqlParameter p in sqcmd.Parameters)
            {
                Debug.WriteLine(string.Format("{0}={1} ({2})",
                    p.ParameterName, p.SqlValue, p.SqlDbType.ToString()));
            }

            return;
        }

        public bool SelectAll(string table, out SqlDataReader reader)
        {
            return Select(table, out reader, "");
        }

        public bool Select(string table, out SqlDataReader reader, string where)
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandText = String.Format("select * from {0} {1}", table, where);
            return Select(sqcmd, out reader);
        }

        public bool Select(SqlCommand sqcmd, out SqlDataReader reader)
        {
            reader = null;
            try
            {
                sqcmd.Connection = conn_;
                DumpSqlCommand(sqcmd);
                reader = sqcmd.ExecuteReader();
                //@@@ we need to close myCommand after reader finished i think?
            }
            catch (Exception e)
            {
                return Crap(e);
            }

            return true;
        }

        public int ExecuteRaw(SqlCommand sqcmd)
        {
            sqcmd.Connection = conn_;
            //DumpSqlCommand(sqcmd);
            return sqcmd.ExecuteNonQuery();
        }
    }
}
