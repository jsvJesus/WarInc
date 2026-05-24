using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace WOAdmin
{
    /*
    ID	int	Unchecked
    EventID	int	Unchecked
    Date	datetime	Unchecked
    IP	varchar(64)	Unchecked
    CustomerID	int	Unchecked
    EventData	varchar(256)	Unchecked
    */

    public class DBSecurityLogRO : SQLRec
    {
        public SQLField EventID = new SQLField("EventID");
        public SQLField Date = new SQLField("Date");
        public SQLField IP = new SQLField("IP");
        public SQLField CustomerID = new SQLField("CustomerID");
        public SQLField EventData = new SQLField("EventData");

        public static string GET_TABLE_NAME()
        {
            return "SecurityLog";
        }

        public DBSecurityLogRO()
        {
            //this class is read only
            //RECORD_ID = new SQLField("ID");

            RegisterField(EventID);
            RegisterField(Date);
            RegisterField(IP);
            RegisterField(CustomerID);
            RegisterField(EventData);
        }
    }
}
