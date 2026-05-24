using System;
using System.Collections.Generic;
using System.Text;
using System.Xml;
using System.Xml.Serialization;
using System.Net;
using System.IO;
using System.Windows.Forms;
using System.Data.SqlClient;

namespace NewsletterSender
{
    class OptOutFromSendGrid
    {
        string OptOutSqlFile;
        System.IO.StreamWriter SW2;

        string SENDGRID_API = "https://sendgrid.com/api/";
        string SENDGRID_KEY = "?api_user=pr@arktosentertainment.com&api_key=wsxmko!10";

        SQLBase sql;
        Form1 form;

        public OptOutFromSendGrid(Form1 in_form)
        {
            form = in_form;
            OptOutSqlFile = string.Format("optout_{0}.sql", DateTime.Now.Ticks);
            SW2 = System.IO.File.CreateText(OptOutSqlFile);
        }

        void DebugLog(string msg)
        {
            form.Invoke(new Form1.TcbDebugLog(form.DebugLog), msg);
        }

        public static Byte[] StringToUTF8ByteArray(String pXmlString)
        {
            UTF8Encoding encoding = new UTF8Encoding();
            Byte[] byteArray = encoding.GetBytes(pXmlString);
            return byteArray;
        }

        string GetWebPage(string url)
        {
            string result = null;

            WebClient client = new WebClient();
            result = client.DownloadString(url);
            return result;
        }

        void OptOutEntries(SendGridXML.entry[] Entries, int OptOutCode)
        {
            if (Entries == null)
                return;

            DebugLog(string.Format("{0} emails\n", Entries.Length));

            //int cnt = 0;
            foreach (SendGridXML.entry e in Entries)
            {
                if (e.email == null)
                    continue;

                try
                {
                    if(e.email.Contains("'"))
                        continue;
                    SW2.WriteLine(string.Format(
                        "update AccountInfo set OptOut1={0} where email='{1}' and OptOut1=0",
                        OptOutCode,
                        e.email));

                    /*
                    SqlCommand sqcmd = new SqlCommand();
                    sqcmd.CommandText = String.Format(
                        "update AccountInfo set OptOut1={0} where email=@email and OptOut1=0",
                        OptOutCode);
                    sqcmd.Parameters.AddWithValue("@email", e.email);
                    sql.ExecuteRaw(sqcmd);

                    cnt++;
                    if((cnt % 1000) == 0)
                        DebugLog(string.Format("{0:00000}/{1:00000}\n", cnt, Entries.Length));
                    */
                }
                catch (Exception _e)
                {
                    DebugLog(string.Format("SQL: {0}\n", _e.Message));
                }
            }
        }

        T GetList<T>(string what, string getApi)
        {
            DebugLog(string.Format("{0}\n", what));

            string url = string.Format("{0}{1}{2}",
                SENDGRID_API,
                getApi,
                SENDGRID_KEY);

            string ans = GetWebPage(url);

            XmlSerializer xs = new XmlSerializer(typeof(T));
            MemoryStream memoryStream = new MemoryStream(StringToUTF8ByteArray(ans));
            return (T)xs.Deserialize(memoryStream);
        }

        void DeleteList(string what, string deleteApi)
        {
            // delete list
            string url = string.Format("{0}{1}{2}",
                SENDGRID_API,
                deleteApi,
                SENDGRID_KEY);
            string ans = GetWebPage(url);

            DebugLog(string.Format("{0} finish\n", what));
        }

        public void DoWork()
        {
            sql = new SQLBase();
            sql.Connect();

            string what = "";

            // bounces
            what = "Unsubscribes";
            SendGridXML.unsubscribes data1 = GetList<SendGridXML.unsubscribes>(
                what, "unsubscribes.get.xml");
            OptOutEntries(data1.Items, 1);
            DeleteList(what, "unsubscribes.delete.xml");

            // invalid emails
            what = "Invalid Emails";
            SendGridXML.invalidemails data2 = GetList<SendGridXML.invalidemails>(
                what, "invalidemails.get.xml");
            OptOutEntries(data2.Items, 2);
            DeleteList(what, "invalidemails.delete.xml");

            // spamreports
            what = "Spam Reports";
            SendGridXML.spamreports data3 = GetList<SendGridXML.spamreports>(
                what, "spamreports.get.xml");
            OptOutEntries(data3.Items, 3);
            DeleteList(what, "spamreports.delete.xml");

            sql = null;

            SW2.Close();
            System.Diagnostics.Process.Start(OptOutSqlFile);
        }

    }
}
