using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Net;
using System.Net.Mail;
using System.Threading;

namespace NewsletterSender
{
    public partial class Form1 : Form
    {
        Thread workThread;
        string emailText = "";
        string emailSubject = "War Inc Battlezone Newsletter";
        int sendMode_ = 0;
        List<WOUser> users_ = new List<WOUser>();

        public Form1()
        {
            InitializeComponent();
            textBox1.MaxLength = 100000000;
            textBox2.MaxLength = 100000000;
            textBox3.MaxLength = 100000000;
            textBox4.Text = emailSubject;
        }

        public delegate void TcbDebugLog(string msg);
        public void DebugLog(string msg)
        {
            textBox2.AppendText(msg);

            //Invalidate();
            //Update();
        }

        delegate void TcbSetFinished();
        void SetFinished()
        {
            DebugLog("-- finished\n");

            button1.Enabled = true;
            button2.Enabled = true;
            button3.Enabled = true;
        }

        SmtpClient mailClient_ = null;
        public void CreateMailClient()
        {
            // Create new SmtpClient Object
            mailClient_ = new SmtpClient("smtp.sendgrid.net", 25);
            mailClient_.UseDefaultCredentials = false;
            mailClient_.Credentials = new NetworkCredential("membersmailwarinc", "wsxmko!10");
        }

        public bool SendMail(List<WOUser> usrs, ref int startIdx)
        {
            return true;
//             for (int i = 0; i < usrs.Count; i++)
//             {
//                 string lastEmail = usrs[i].email;
//                 MessageBox.Show(lastEmail);
//             }
//                 
//             if (emailText.Length < 10)
//                 return false;
// 
//             string from = "Online Warmongers Group <news@news.membersmailwarinc.com>";
// 
//             for (int curTry = 0; curTry < 20; curTry++)
//             {
//                 try
//                 {
//                     // Create new MailMessage Object
//                     MailMessage message = new MailMessage(from, "donotreply@email.news-thewarinc.com", emailSubject, emailText);
//                     message.IsBodyHtml = true;
// 
//                     const int MAX_BCC_TO_SEND = 200;
//                     MailAddressCollection bcc = message.Bcc;
// 
//                     int curEmail = startIdx;
//                     for (; curEmail < usrs.Count && curEmail < (startIdx + MAX_BCC_TO_SEND); curEmail++)
//                     {
//                         string lastEmail = usrs[curEmail].email;
//                         System.Diagnostics.Debug.WriteLine(lastEmail);
//                         try
//                         {
//                             bcc.Add(new MailAddress(lastEmail));
//                         }
//                         catch (Exception/* e1*/)
//                         {
//                             this.Invoke(new TcbDebugLog(DebugLog), string.Format("BAD({0}) ", lastEmail));
//                         }
//                     }
// 
//                     mailClient_.Send(message);
// 
//                     startIdx = curEmail;
//                     return true;
//                 }
//                 catch (Exception e)
//                 {
//                     this.Invoke(new TcbDebugLog(DebugLog), string.Format("{0}, try{1}\n", e.Message, curTry));
//                 }
//             }
// 
//             return false;
        }

        private void SendMail_WorkerThread()
        {
            try
            {
                if (emailText.Length < 10)
                    return;

                CreateMailClient();

                if (sendMode_ == 0) // test send
                {
                    users_ = new List<WOUser>();
                    //users_.Add(new WOUser("alindyar@mail.ru"));
                    users_.Add(new WOUser("alindyar@gmail.com"));
                    users_.Add(new WOUser("denis.zhulitov@arktosentertainment.com"));
                    users_.Add(new WOUser("sergey.titov@arktosentertainment.com"));
                    users_.Add(new WOUser("titov@voidproduction.com"));
                }
                else if(sendMode_ == 1) // send to all
                {
                    users_ = new List<WOUser>();

                    this.Invoke(new TcbDebugLog(DebugLog), "reading users from SQL\n");
                    SQLBase sql = new SQLBase();
                    sql.Connect();

                    this.Invoke(new TcbDebugLog(DebugLog), "done\n");
                    

                    System.Data.SqlClient.SqlDataReader reader;
                    sql.SelectAll("AccountInfo", out reader);

                    while (reader.Read())
                    {
                        string optout1 = reader["OptOut1"].ToString();
                        if (optout1.Equals("0") == false)
                            continue;

                        WOUser user = new WOUser();
                        user.email = reader["email"].ToString();
                        user.first = reader["firstname"].ToString();
                        user.last = reader["lastname"].ToString();
                        users_.Add(user);
                    }
                    reader.Close();
                }
                else if (sendMode_ == 2) // send to specified
                {
                }

//                 this.Invoke(new TcbDebugLog(DebugLog),
//                     string.Format("{0} emails\n", users_.Count));
// 

                for (int i = 0; i < users_.Count; i++)
                {

                    string lastEmail = users_[i].email;
                        MessageBox.Show(lastEmail);
                   
//                     this.Invoke(new TcbDebugLog(DebugLog),
//                         string.Format("[{0}/{1}] - ", i, users_.Count));
// 
//                     SendMail(users_, ref i);
//                     this.Invoke(new TcbDebugLog(DebugLog), " ok\n");
                }
            }
            catch (ThreadAbortException)
            {
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString(), "crash");
                return;
            }

            this.Invoke(new TcbSetFinished(SetFinished), new object[] {});
        }

        private void OptOut_WorkerThread()
        {
            try
            {
                this.Invoke(new TcbDebugLog(DebugLog),
                    string.Format("Starting opting out sendgrid emails\n"));

                OptOutFromSendGrid oo = new OptOutFromSendGrid(this);
                oo.DoWork();
            }
            catch (ThreadAbortException)
            {
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString(), "crash");
                return;
            }

            this.Invoke(new TcbSetFinished(SetFinished), new object[] { });
        }

        private void FilterBadEmails_WorkerThread()
        {
            try
            {
                this.Invoke(new TcbDebugLog(DebugLog),
                    string.Format("Starting opting out bad emails\n"));

                string OptOutSqlFile = string.Format("optout_{0}.sql", DateTime.Now.Ticks);
                System.IO.StreamWriter SW2 = System.IO.File.CreateText(OptOutSqlFile);

                users_ = new List<WOUser>();

                this.Invoke(new TcbDebugLog(DebugLog), "reading users from SQL\n");
                SQLBase sql = new SQLBase();
                sql.Connect();

                System.Data.SqlClient.SqlDataReader reader;
                sql.SelectAll("AccountInfo", out reader);

                while (reader.Read())
                {
                    string optout1 = reader["OptOut1"].ToString();
                    if (optout1.Equals("0") == false)
                        continue;

                    string email = reader["email"].ToString();

                    try
                    {
                        MailAddress mail = new MailAddress(email);
                        mail = null;
                    }
                    catch (Exception/* e1*/)
                    {
                        //this.Invoke(new TcbDebugLog(DebugLog), string.Format("BAD({0}) ", email));

                        email = email.Replace("'", "''");
                        SW2.WriteLine(string.Format(
                            "update AccountInfo set OptOut1=10 where email='{0}' and OptOut1=0",
                            email));
                    }
                }
                reader.Close();

                SW2.Close();
                System.Diagnostics.Process.Start(OptOutSqlFile);
            }
            catch (ThreadAbortException)
            {
            }
            catch (Exception e)
            {
                MessageBox.Show(e.ToString(), "crash");
                return;
            }

            this.Invoke(new TcbSetFinished(SetFinished), new object[] { });
        }

        void StartWork(int mode)
        {
            emailSubject = textBox4.Text;
            if (emailSubject.Length < 5)
            {
                MessageBox.Show("need email subject");
                return;
            }

            emailText = textBox1.Text;
            if (emailText.Length < 10)
            {
                MessageBox.Show("paste newsletter text to left pane");
                return;
            }

            sendMode_ = mode;
            workThread = new Thread(new ThreadStart(SendMail_WorkerThread));
            workThread.Start();

            button1.Enabled = false;
            button2.Enabled = false;
            button3.Enabled = false;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            StartWork(0);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            StartWork(1);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            string emails = textBox3.Text;
            if (emails.Length < 2)
            {
                MessageBox.Show("paste emails to right pane");
                return;
            }
            users_ = new List<WOUser>();

            emails.Replace('\r', ' ');
            string[] arr = emails.Split("\n ,".ToCharArray());
            for(int i=0; i<arr.Length; i++)
            {
                arr[i] = arr[i].Trim();
                if (arr[i].Length < 2) 
                    continue;

                users_.Add(new WOUser(arr[i]));
            }

            StartWork(2);
        }

        private void button4_Click(object sender, EventArgs e)
        {
            //workThread = new Thread(new ThreadStart(OptOut_WorkerThread));
            workThread = new Thread(new ThreadStart(FilterBadEmails_WorkerThread));
            workThread.Start();

            button1.Enabled = false;
            button2.Enabled = false;
            button3.Enabled = false;
        }

        private void Form1_Load(object sender, EventArgs e)
        {

        }
    }
}
