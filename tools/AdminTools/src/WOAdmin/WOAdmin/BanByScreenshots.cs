using System;
using System.Collections.Generic;
using System.Text;

namespace WOAdmin
{
    class BanByScreenshots
    {
        public void DoIt(string path)
        {
            Dictionary<int, int> users = new Dictionary<int, int>();

            string[] files = System.IO.Directory.GetFiles(path, "*.jpg");
            foreach(string file in files)
            {
                // fname in form GS_1cc625ebba7a005_1288443711_6497d172.jpg
                string strCustomerID = file.Substring(19 + path.Length, 10);
                int CustomerID = Convert.ToInt32(strCustomerID);

                int temp;
                if (users.TryGetValue(CustomerID, out temp) == false)
                {
                    users.Add(CustomerID, 1);
                    BanUser(CustomerID);
                }
            }
        }

        void Log(string msg)
        {
            System.Diagnostics.Debug.WriteLine(msg);
        }

        void BanUser(int CustomerID)
        {
            WOUsers users_ = Form1.users_;
            users_.ReadUsersFromDB("LoginID.CustomerID", "=", CustomerID.ToString());
            if (users_.data_.Values.Count == 0)
            {
                Log("CustomerID NOT FOUND: " + CustomerID.ToString());
                return;
            }

            foreach (DBUser rec in users_.data_.Values)
            {
                if (rec.AccountStatus.AsInt() >= 200)
                {
                    Log(string.Format("CustomerID {0} already banned", CustomerID));
                    return;
                }

                string banReason = string.Format("{0}-{1} wallhack", DateTime.Now.Month, DateTime.Now.Day);

                rec.AccountStatus.v_int = 200;
                if(rec.refl_AccountInfo.admin_note.v_string.Length > 0)
                    rec.refl_AccountInfo.admin_note.v_string = banReason + ", " + rec.refl_AccountInfo.admin_note.v_string;
                else
                    rec.refl_AccountInfo.admin_note.v_string = banReason;

                Log(string.Format("Banning CustomerID {0}, {1}", CustomerID, rec.AccountName.v_string));
                Form1.sql.Update(rec);
                Form1.sql.Update(rec.refl_AccountInfo);

                users_.AddToListView(rec);
            }
        }
    }
}
