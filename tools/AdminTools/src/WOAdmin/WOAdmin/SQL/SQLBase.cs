using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;
using System.Diagnostics;

namespace WOAdmin
{
    public class SQLBase
    {
        public string user_ = "sa";
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

        public bool Connect(int serverId, string user, string pass)
        {
            string server = "31.132.151.4";
            string workdb = "warinc";
            switch (serverId)
            {
                case 0:
                    server = "192.95.7.127,4685";
                    break;
                case 1:
                    server = "192.95.7.127,4685";
                    break;
                default:
                    throw new ArgumentException("bad ServerId");
            }

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

            user_ = user;
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
            return SelectWhere(table, out reader, "");
        }

        public bool SelectWhere(string table, out SqlDataReader reader, string where)
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

        public bool InsertNew(SQLRec rec)
        {
            try
            {
                if (rec.TABLE_ID == null)
                    throw new ArgumentException("no TABLE_ID");

                SqlCommand sqcmd = new SqlCommand();

                string fields = "";
                string values = "";
                rec.BuildInsertString(ref fields, ref values, sqcmd.Parameters);

                sqcmd.CommandText = string.Format("INSERT INTO {0} ({1}) VALUES ({2})",
                    rec.TABLE_ID, fields, values);
                sqcmd.Connection = conn_;
                DumpSqlCommand(sqcmd);
                sqcmd.ExecuteNonQuery();
                sqcmd.Dispose();

                // update WOAdmin Logs
                DBWOAdminChange.LogInsertRecord(this, rec);
            }
            catch (Exception e)
            {
                return Crap(e);
            }
            return true;
        }

        public bool RefreshData(SQLRec rec)
        {
            SqlDataReader reader = null;

            try
            {
                if (rec.TABLE_ID == null)
                    throw new ArgumentException("no TABLE_ID");

                // read current data from DB
                SqlCommand sqcmd = new SqlCommand();
                sqcmd.CommandText = String.Format(
                    "select * from {0} where {1}={2}",
                    rec.TABLE_ID, rec.RECORD_ID.name, rec.RECORD_ID.ToString());

                sqcmd.Connection = conn_;
                DumpSqlCommand(sqcmd);
                reader = sqcmd.ExecuteReader();
                reader.Read();
                if (!reader.HasRows)
                {
                    MessageBox.Show("can't update record - it was already deleted");
                    return false;
                }

                rec.Fill(reader);
            }
            catch (Exception e)
            {
                return Crap(e);
            }
            finally
            {
                if (reader != null)
                    reader.Close();
            }
            return true;
        }

        bool IsPermittedToOverwrite(SQLRec recO)
        {
            try
            {
                SQLRec recN = recO.CloneRecWithoutData();
                if (!RefreshData(recN))
                    return false;

                string failMsg = "Warning: values in DB was already changed.\n\nYou will be overwriting following fields:\n";
                bool isFail = false;

                for (int i = 0; i < recO.allFields_.Count; i++)
                {
                    var fld1 = recO.allFields_[i];
                    var fld2 = recN.allFields_[i];
                    if (!fld1.IsEdited())
                        continue;

                    string str1 = fld1.stored_value.ToString();
                    string str2 = fld2.ToObject().ToString();
                    // ignore datetime - there is problem in datetime format
                    if (str1 != str2 && fld1.type2 != SQLDataType.DATETIME)
                    {
                        isFail = true;
                        failMsg += string.Format(
                            "  {0} actual value {1} with {2}\n",
                            fld1.name, str2, fld1.ToObject().ToString()
                            );

                        // replace, so DBWOAdminChange will track it.
                        fld1.stored_value = fld2.stored_value;
                    }
                }
                if (!isFail)
                    return true;

                failMsg += "\nARE YOU SURE ABOUT IT?";
                var res = MessageBox.Show(
                    failMsg,
                    "Warning: Overwriting DB values",
                    MessageBoxButtons.YesNo,
                    MessageBoxIcon.Warning);
                return res == DialogResult.Yes ? true : false;
            }
            catch (Exception e)
            {
                MessageBox.Show(e.Message);
                return false;
            }
        }

        public bool Update(SQLRec rec)
        {
            try
            {
                if (rec.TABLE_ID == null)
                    throw new ArgumentException("no TABLE_ID");

                if (!IsPermittedToOverwrite(rec))
                    return false;

                SqlCommand sqcmd = new SqlCommand();

                string sets = "";
                string where = "";
                rec.BuildUpdateString(ref sets, sqcmd.Parameters);
                rec.BuildWhereString(ref where);
                if (sets == "")
                {
                    // nothing to update
                    return true;
                }

                sqcmd.CommandText = string.Format("UPDATE {0} SET {1} WHERE {2}",
                    rec.TABLE_ID, sets, where);
                sqcmd.Connection = conn_;
                DumpSqlCommand(sqcmd);
                sqcmd.ExecuteNonQuery();
                sqcmd.Dispose();

                // update WOAdmin Logs
                DBWOAdminChange.LogUpdateRecord(this, rec);
            }
            catch (Exception e)
            {
                return Crap(e);
            }
            return true;
        }

        public bool Delete(SQLRec rec)
        {
            try
            {
                if (rec.TABLE_ID == null)
                    throw new ArgumentException("no TABLE_ID");

                SqlCommand sqcmd = new SqlCommand();

                string where = "";
                rec.BuildWhereString(ref where);

                sqcmd.CommandText = string.Format("DELETE FROM {0} WHERE {1}",
                    rec.TABLE_ID, where);
                sqcmd.Connection = conn_;
                DumpSqlCommand(sqcmd);
                sqcmd.ExecuteNonQuery();
                sqcmd.Dispose();

                // update WOAdmin Logs
                DBWOAdminChange.LogDeleteRecord(this, rec);
            }
            catch (Exception e)
            {
                return Crap(e);
            }
            return true;
        }

        public bool Exec(string cmd, bool addToWOLog)
        {
            try
            {
                SqlCommand sqcmd = new SqlCommand();
                sqcmd.CommandText = cmd;
                sqcmd.Connection = conn_;
                DumpSqlCommand(sqcmd);
                sqcmd.ExecuteNonQuery();
                sqcmd.Dispose();

                if (addToWOLog)
                {
                    // update WOAdmin Logs
                    DBWOAdminChange.LogExecRecord(this, cmd);
                }
            }
            catch (Exception e)
            {
                return Crap(e);
            }
            return true;
        }
    }
}
