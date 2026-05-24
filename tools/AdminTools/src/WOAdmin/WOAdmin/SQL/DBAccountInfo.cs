using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
/*
CustomerID	int	Unchecked
* * */

    public class DBAccountInfo : SQLRec
    {
        public SQLField email = new SQLField("email");
        public SQLField admin_note = new SQLField("admin_note");

        public static string GET_TABLE_NAME()
        {
            return "AccountInfo";
        }

        public DBAccountInfo()
        {
            TABLE_ID = GET_TABLE_NAME();
            RECORD_ID = new SQLField("CustomerID");

            RegisterField(email);
            RegisterField(admin_note);

            // setup varchar values
            email.VARCHAR_LEN = 100;
            admin_note.VARCHAR_LEN = 128;
        }

        public void SetNewRecordDefaults()
        {
            email.FromObject("");
            admin_note.FromObject("");
        }
    }
}
