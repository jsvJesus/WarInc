using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class Form1 : Form
    {
        static public Form1 THIS = null;

        // global vars for database
        static public WOWeapons weapons_ = new WOWeapons();
        static public WOGears gears_ = new WOGears();
        static public WOItems items_ = new WOItems();
        static public WOPackages packages_ = new WOPackages();
        static public WOAttachments attachments_ = new WOAttachments();
        static public WOUsers users_ = new WOUsers();
        static public WOClans clans_ = new WOClans();

        enum EViews : uint
        {
            None,
            Home,
            Users,
            Items,
            ItemMall,
            Servers,
            Transations,
            Partners,
            Coupons,
            Clans,
            MAX,
        };
        EViews currentView_ = EViews.None;
        Form[] views_ = new Form[(int)EViews.MAX];
        GSButton[] vbuttons_ = new GSButton[(int)EViews.MAX];

        static public SQLBase sql = new SQLBase();

        public Form1()
        {
            THIS = this;

            InitializeComponent();
            panelView.Hide();
            this.Size = new Size(1280, 1000);

            CreateView(EViews.Home, new ViewHome(), button1);
            CreateView(EViews.Users, new ViewUsers(), button2);
            CreateView(EViews.Items, new ViewItems(), button3);
            //CreateView(EViews.Servers, new ViewServers(), button5);
            CreateView(EViews.Transations, new ViewTransactions(), button7);
            //CreateView(EViews.Coupons, new ViewCoupons(), button9);
            CreateView(EViews.Clans, new ViewClans(), button4);

            SwitchToView(EViews.Home);
        }

        void CreateView(EViews idx, Form frm, GSButton btn)
        {
            frm.TopLevel = false;
            frm.FormBorderStyle = FormBorderStyle.None;
            frm.Location = panelView.Location;
            frm.Size = panelView.Size;
            frm.Anchor = panelView.Anchor;
            frm.Hide();

            views_[(int)idx] = frm;
            this.Controls.Add(frm);
            vbuttons_[(int)idx] = btn;
        }

        void SwitchToView(EViews idx)
        {
            if (currentView_ == idx)
                return;

            if(currentView_ != EViews.None)
            {
                views_[(int)currentView_].Hide();
                (views_[(int)currentView_] as IAdminView).OnViewDeactivated();
                vbuttons_[(int)currentView_].StaySelected = false;
                Invalidate();
            }

            currentView_ = idx;
            views_[(int)currentView_].Show();
            (views_[(int)currentView_] as IAdminView).OnViewActivated();
            vbuttons_[(int)currentView_].StaySelected = true;
        }

        void GeneratePromoCodesSingle(int numbersToCreate, string prefix, int code)
        {
            GeneratePromoCodesEx(false, numbersToCreate, prefix, code);
        }

        void GeneratePromoCodesMulti(string prefix, int code)
        {
            GeneratePromoCodesEx(true, 1, prefix, code);
        }

        void GeneratePromoCodesEx(bool multiuse, int numbersToCreate, string prefix, int code)
        {
            System.IO.StreamWriter SW1 = new System.IO.StreamWriter(string.Format("\\WOCoupons_{0}.sql", prefix), true);
            SW1.WriteLine("use gameid_v1");

            System.IO.StreamWriter SW2 = System.IO.File.CreateText(string.Format("\\WOCoupons_{0}.txt", prefix));

            for (int codes = 0; codes < numbersToCreate; )
            {
                //string key = RandomSNKGenerator.GetUnique7SizeKey();
                string key = RandomSNKGenerator.GetUniqueKey();

                string sqcmd = string.Format("insert into Coupons2 values('{0}', 0, 0, {1}, {2})",
                    key, code, multiuse ? 1 : 0);
                //if (!sql.Exec(sqcmd))
                //    continue;

                SW1.WriteLine(sqcmd);
                SW2.WriteLine(key);
                codes++;
            }
            SW1.Close();
            SW2.Close();
        }

        void AddUniqueInviteKey(int ReferralID)
        {
            string key = RandomSNKGenerator.GetUniqueKey();
            string sqcmd = string.Format("INSERT INTO InviteCodes (invitecode, ReferralID) VALUES ('{0}', '{1}')",
                key.Trim(),
                ReferralID);

            if (!sql.Exec(sqcmd, false))
                return;

            //System.IO.StreamWriter SW;
            //SW = System.IO.File.AppendText("C:\\WOKeys.txt");
            //SW.WriteLine(key);
            //SW.Close();
        }

        private void Form1_Shown(object sender, EventArgs e)
        {
            Application.DoEvents();

            string user = "WOAdmin";
            string password = "";
            int serverId = 0;

            //GeneratePromoCodesSingle(15000, "15,000 x 7500GC", 52);
            //GeneratePromoCodesMulti("devEvent", 200);

            //password = "frtV4zAqo"; //@
            //serverId = 1;
            //password = "wsxplm20"; //@
            //serverId = 0;

            if (password.Length == 0)
            {
                EnterPassword pwd = new EnterPassword();
                pwd.ShowDialog(this);
                user = pwd.tbUser.Text;
                password = pwd.tbPasswd.Text;
                serverId = pwd.comboBox1.SelectedIndex;
                if (password.Length == 0 || user.Length == 0)
                {
                    Close();
                    return;
                }
                pwd.Dispose();
            }

            if (!sql.Connect(serverId, user, password))
            {
                Close();
                return;
            }

            //weapons_.ReadFromDB();
            //var ddd = new EditLootDrop(301108, "UberBox");
            //ddd.Show();
        }

        public bool GetItemInfo(int key, ref string name, ref string desc)
        {
            int itemCat = 0;
            return GetItemInfo(key, ref name, ref desc, ref itemCat);
        }

        public DBCommonItem GetItemRec(int key)
        {
            DBWeapon wpn = weapons_.SearchByKey(key);
            if (wpn != null)
                return wpn;

            DBGear gear = gears_.SearchByKey(key);
            if (gear != null)
                return gear;

            DBItem item = items_.SearchByKey(key);
            if (item != null)
                return item;

            DBPackage package = packages_.SearchByKey(key);
            if (package != null)
                return package;

            return null;
        }

        public bool GetItemInfo(int key, ref string name, ref string desc, ref int cat)
        {
            DBCommonItem rec = GetItemRec(key);
            if(rec != null)
            {
                name = rec.Name.ToString();
                desc = rec.Description.ToString();
                cat = rec.Category.AsInt();
                return true;
            }

            name = string.Format("<{0}>", key);
            desc = "<?>";
            cat = 0;
            return false;
        }

        public static string GetUserName(int key)
        {
            DBUser usr = users_.SearchByKey(key);
            if (usr != null)
            {
                return usr.AccountName.ToString();
            }
            return key.ToString();
        }

        private void button6_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            SwitchToView(EViews.Home);
        }

        private void button2_Click(object sender, EventArgs e)
        {
            SwitchToView(EViews.Users);
        }

        private void button3_Click(object sender, EventArgs e)
        {
            SwitchToView(EViews.Items);
        }

        private void button4_Click(object sender, EventArgs e)
        {
        }

        private void button5_Click(object sender, EventArgs e)
        {
            SwitchToView(EViews.Servers);
        }

        private void button7_Click(object sender, EventArgs e)
        {
            SwitchToView(EViews.Transations);
        }

        private void button8_Click(object sender, EventArgs e)
        {
        }

        private void button9_Click(object sender, EventArgs e)
        {
            //SwitchToView(EViews.Coupons);
        }

        private void button4_Click_1(object sender, EventArgs e)
        {
            SwitchToView(EViews.Clans);
        }
    }
}
