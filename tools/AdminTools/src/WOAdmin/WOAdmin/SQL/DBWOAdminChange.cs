using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
RecordID	int	Unchecked
ChangeTime	datetime	Checked
UserName	varchar(64)	Checked
Action	int	Checked
Field	varchar(512)	Checked
OldValue	varchar(2048)	Checked
NewValue	varchar(2048)	Checked
    */

    public class DBWOAdminChange : SQLRec
    {
        public SQLField ChangeTime = new SQLField("ChangeTime");
        public SQLField UserName = new SQLField("UserName");
        public SQLField Action = new SQLField("Action");
        public SQLField RecordID = new SQLField("RecordID");
        public SQLField Field = new SQLField("Field");
        public SQLField OldValue = new SQLField("OldValue");
        public SQLField NewValue = new SQLField("NewValue");

        public static string GET_TABLE_NAME()
        {
            return "DBG_WOAdminChanges";
        }

        public DBWOAdminChange()
        {
            RECORD_ID = new SQLField("ChangeID");
            RECORD_ID.type2 = SQLDataType.NONE;

            TABLE_ID = GET_TABLE_NAME();

            RegisterField(ChangeTime);
            RegisterField(UserName);
            RegisterField(Action);
            RegisterField(RecordID);
            RegisterField(Field);
            RegisterField(OldValue);
            RegisterField(NewValue);
        }

        static string user_ = null;
        static string GetUser()
        {
            if (user_ == null)
            {
                user_ = System.Environment.MachineName + "\\" + System.Environment.UserName;
                if (user_.Length > 63) user_ = user_.Substring(0, 63);
            }

            return user_;
        }

        public static void LogExecRecord(SQLBase sql, string sqcmd)
        {
            DBWOAdminChange upd = new DBWOAdminChange();
            upd.ChangeTime.FromObject(DateTime.Now);
            upd.UserName.FromObject(GetUser());
            upd.Action.FromObject(3);

            string insertData = sqcmd;
            if (insertData.Length > 1023) insertData = insertData.Remove(1023);

            upd.RecordID.FromObject(0);
            upd.Field.FromObject("");
            upd.OldValue.FromObject("");
            upd.NewValue.FromObject(insertData);

            sql.InsertNew(upd);
        }

        public static void LogInsertRecord(SQLBase sql, SQLRec rec)
        {
            // do not log inserting of log entries
            if ((rec as DBWOAdminChange) != null)
                return;

            DBWOAdminChange upd = new DBWOAdminChange();
            upd.ChangeTime.FromObject(DateTime.Now);
            upd.UserName.FromObject(GetUser());
            upd.Action.FromObject(0);

            // create insert string
            string insertData = "";
            foreach (SQLField fld in rec.allFields_)
            {
                insertData += string.Format("{0}={1}, ", fld.name, fld.ToString());
            }
            if (insertData.Length > 1023) insertData = insertData.Remove(1023);

            upd.RecordID.FromObject(0);
            upd.Field.FromObject(string.Format("{0}", rec.TABLE_ID));
            upd.OldValue.FromObject("");
            upd.NewValue.FromObject(insertData);

            sql.InsertNew(upd);
        }

        public static void LogDeleteRecord(SQLBase sql, SQLRec rec)
        {
            DBWOAdminChange upd = new DBWOAdminChange();
            upd.ChangeTime.FromObject(DateTime.Now);
            upd.UserName.FromObject(GetUser());
            upd.Action.FromObject(2);

            upd.RecordID.FromObject(rec.RECORD_ID.AsInt());
            upd.Field.FromObject(string.Format("{0}", rec.TABLE_ID));
            upd.OldValue.FromObject("");
            upd.NewValue.FromObject("");

            sql.InsertNew(upd);
        }

        public static void LogUpdateRecord(SQLBase sql, SQLRec rec)
        {
            DBWOAdminChange upd = new DBWOAdminChange();
            upd.ChangeTime.FromObject(DateTime.Now);
            upd.UserName.FromObject(GetUser());
            upd.Action.FromObject(1);

            foreach (SQLField fld in rec.allFields_)
            {
                if (!fld.IsEdited())
                    continue;

                upd.RecordID.FromObject(rec.RECORD_ID.AsInt());
                upd.Field.FromObject(string.Format("{0}.{1}", rec.TABLE_ID, fld.name));
                upd.OldValue.FromObject(fld.stored_value.ToString());
                upd.NewValue.FromObject(fld.ToObject().ToString());

                sql.InsertNew(upd);

                // and update stored value
                fld.stored_value = fld.ToObject();
            }
        }
    }
}
