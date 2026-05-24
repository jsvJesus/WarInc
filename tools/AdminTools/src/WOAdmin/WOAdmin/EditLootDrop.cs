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
    public partial class EditLootDrop : Form
    {
        int MysteryCrateID_ = 0;
        List<LootEntry> loots_ = new List<LootEntry>();

        public EditLootDrop(int in_BoxID, string in_name)
        {
            InitializeComponent();

            MysteryCrateID_ = in_BoxID;
            lblLootId.Text = string.Format("{0} : {1}", MysteryCrateID_, in_name);
            this.Text = lblLootId.Text;

            LoadFromDB();
            RedrawUI();
        }

        static public int GetItemsCount(int LootID)
        {
            EditLootDrop ld = new EditLootDrop(LootID, "");
            int items = ld.loots_.Count;
            ld.Dispose();
            return items;
        }

        void LoadFromDB()
        {
            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandText = string.Format("select * from {0} where LootID={1}",
                DBLootEntry.GET_TABLE_NAME(),
                MysteryCrateID_);

            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                DBLootEntry rec = new DBLootEntry();
                rec.Fill(reader);

                LootEntry le = new LootEntry(rec);
                loots_.Add(le);
            }
            reader.Close();
        }

        void SaveToDB()
        {
            string cmd = string.Format("delete from {0} where LootID={1}",
                DBLootEntry.GET_TABLE_NAME(),
                MysteryCrateID_);
            Form1.sql.Exec(cmd, true);

            foreach (LootEntry le in loots_)
            {
                le.rec.RECORD_ID.type2 = SQLDataType.NONE; // clear record id
                le.rec.LootID.FromObject(MysteryCrateID_);
                Form1.sql.InsertNew(le.rec);
            }
        }

        Double GetItemPriceByDay(DBCommonItem rec, int days)
        {
            int price = 0;
            double coef = 1.0f;
            if (days <= 1)
            {
                price = rec.Price1.AsInt() > 0 ? rec.Price1.AsInt() : 850;
                coef = (double)days / 1;
            }
            else if (days <= 7)
            {
                price = rec.Price7.AsInt() > 0 ? rec.Price7.AsInt() : 3400;
                coef = (double)days / 7;
            }
            else if (days <= 30)
            {
                price = rec.Price30.AsInt() > 0 ? rec.Price30.AsInt() : 10000;
                coef = (double)days / 30;
            }
            else
            {
                price = rec.PriceP.AsInt() > 0 ? rec.PriceP.AsInt() : 20000;
                coef = (double)days / 2000;
            }

            return (Double)price * coef;
        }

        Double GetItemPrice(LootEntry le)
        {
            int ItemId = le.rec.ItemID.AsInt();
            if(ItemId == 0)
                return 0;

            DBCommonItem rec = Form1.THIS.GetItemRec(ItemId);
            if (rec == null)
            {
                lblBoxInfo.Text = string.Format("AvgPrice: Item {0} not found", ItemId);
                return -1;
            }

            int days = le.rec.ExpDaysMin.AsInt();
            if (days != le.rec.ExpDaysMax.AsInt())
                days = (le.rec.ExpDaysMin.AsInt() + le.rec.ExpDaysMax.AsInt()) / 2;

            return GetItemPriceByDay(rec, days);
        }

        void RecalculateAvgBoxPrice()
        {
            double avg = 0;
            int errs = 0;
            foreach (LootEntry le in loots_)
            {
                double price = GetItemPrice(le);
                if (price < 0)
                {
                    errs++;
                    continue;
                }
                avg += price * le.nChance;
            }

            if(errs > 0 )
                lblBoxInfo.Text = string.Format("AvgPrice: {0} GP. NOTE: {1} errors", (int)avg, errs);
            else
                lblBoxInfo.Text = string.Format("AvgPrice: {0} GP", (int)avg);
        }

        void RecalculateChances()
        {
            if (loots_.Count == 0)
                return;

            // renormalize chances
            double whole = 0;
            foreach (LootEntry le in loots_)
            {
                whole += le.rec.Chance.v_double;
            }

            foreach (LootEntry le in loots_)
            {
                le.nChance = (whole > 0) ? le.rec.Chance.v_double / whole : 0.0f;
                le.lb1.Text = string.Format("{0:0.000}%", le.nChance * 100);
                le.lb3.Text = string.Format("1/{0}", (int)(1 / le.nChance));
            }

            RecalculateAvgBoxPrice();
        }

        void RedrawUI()
        {
            int x = 0;
            int y = 0;

            panel1.Controls.Clear();

            int[] widths = { 200, 70, 50, 50, 400 };
            string[] descs = { "What (Item/ExpDays/GD)", "Chance", "Real %", "Drop", "Description" };

            for (int i = 0; i < 5; i++)
            {
                Label lb1 = new Label();
                lb1.Location = new System.Drawing.Point(x + 30, y);
                lb1.Size = new System.Drawing.Size(widths[i], 23);
                lb1.Text = descs[i];
                lb1.ForeColor = Color.Blue;
                panel1.Controls.Add(lb1);
                x += widths[i] + 10;
            }
            y += 25;

            // recreate UI elements
            foreach (LootEntry le in loots_)
            {
                x = 0;

                le.DeleteUI();

                Button btn = new Button();
                btn.Location = new System.Drawing.Point(x, y);
                btn.Size = new System.Drawing.Size(23, 23);
                btn.Name = "button1";
                btn.Text = "-";
                btn.Click += new System.EventHandler(this.buttonDelete_Click);
                btn.Tag = le;
                panel1.Controls.Add(btn);
                x += 30;

                if (le.DropType == 0)
                {
                    // GD
                    le.ed1 = new TextBox();
                    le.ed1.Location = new System.Drawing.Point(x, y);
                    le.ed1.Size = new System.Drawing.Size(widths[0], 23);
                    le.ed1.Text = string.Format("{0}-{1}", le.rec.GDMin.v_int, le.rec.GDMax.v_int);
                    le.ed1.TextChanged += new System.EventHandler(this.OnTextChanged_GD);
                    le.ed1.Tag = le;
                    panel1.Controls.Add(le.ed1);
                }
                else
                {
                    // Item with EXP Date
                    le.ed1 = new TextBox();
                    le.ed1.Location = new System.Drawing.Point(x, y);
                    le.ed1.Size = new System.Drawing.Size(60, 23);
                    le.ed1.Text = le.rec.ItemID.ToString();
                    le.ed1.TextChanged += new System.EventHandler(this.OnTextChanged_ItemID);
                    le.ed1.Tag = le;
                    panel1.Controls.Add(le.ed1);

                    le.ed3 = new TextBox();
                    le.ed3.Location = new System.Drawing.Point(x + 70, y);
                    le.ed3.Size = new System.Drawing.Size(60, 23);
                    le.ed3.Text = string.Format("{0}-{1}", le.rec.ExpDaysMin.v_int, le.rec.ExpDaysMax.v_int);
                    le.ed3.TextChanged += new System.EventHandler(this.OnTextChanged_ExpDays);
                    le.ed3.Tag = le;
                    panel1.Controls.Add(le.ed3);

                    le.ed4 = new TextBox();
                    le.ed4.Location = new System.Drawing.Point(x + 140, y);
                    le.ed4.Size = new System.Drawing.Size(60, 23);
                    le.ed4.Text = string.Format("{0}", le.rec.GDIfHave.v_int);
                    le.ed4.TextChanged += new System.EventHandler(this.OnTextChanged_GDIfHave);
                    le.ed4.Tag = le;
                    panel1.Controls.Add(le.ed4);
                }
                x += widths[0] + 10;

                le.ed2 = new TextBox();
                le.ed2.Location = new System.Drawing.Point(x, y);
                le.ed2.Size = new System.Drawing.Size(widths[1], 23);
                le.ed2.Text = le.rec.Chance.ToString();
                le.ed2.TextChanged += new System.EventHandler(this.OnTextChanged_Chance);
                le.ed2.Tag = le;
                panel1.Controls.Add(le.ed2);
                x += widths[1] + 10;

                le.lb1 = new Label();
                le.lb1.Location = new System.Drawing.Point(x, y);
                le.lb1.Size = new System.Drawing.Size(widths[2], 23);
                le.lb1.Text = "0.000%";
                panel1.Controls.Add(le.lb1);
                x += widths[2] + 10;

                le.lb3 = new Label();
                le.lb3.Location = new System.Drawing.Point(x, y);
                le.lb3.Size = new System.Drawing.Size(widths[3], 23);
                le.lb3.Text = le.GetItemDesc();
                panel1.Controls.Add(le.lb3);
                x += widths[3] + 10;

                le.lb2 = new Label();
                le.lb2.Location = new System.Drawing.Point(x, y);
                le.lb2.Size = new System.Drawing.Size(widths[4], 23);
                le.lb2.Text = le.GetItemDesc();
                panel1.Controls.Add(le.lb2);
                x += widths[4] + 10;


                y += 30;
            }

            RecalculateChances();

            // add new button
            {
                x = 30;
                Button btn = new Button();
                btn.Location = new System.Drawing.Point(x, y);
                btn.Size = new System.Drawing.Size(60, 23);
                btn.Name = "button1";
                btn.Text = "+GD";
                btn.Click += new System.EventHandler(this.buttonNewGD_Click);
                panel1.Controls.Add(btn);
                x += 70;

                btn = new Button();
                btn.Location = new System.Drawing.Point(x, y);
                btn.Size = new System.Drawing.Size(60, 23);
                btn.Name = "button1";
                btn.Text = "+Item";
                btn.Click += new System.EventHandler(this.buttonNewItem_Click);
                panel1.Controls.Add(btn);
                x += 120;

                btn = new Button();
                btn.Location = new System.Drawing.Point(x + 20, y);
                btn.Size = new System.Drawing.Size(40, 23);
                btn.Name = "button1";
                btn.Text = "sort";
                btn.Click += new System.EventHandler(this.buttonResort_Click);
                panel1.Controls.Add(btn);
            }

        }

        private void OnTextChanged_ItemID(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            LootEntry le = (LootEntry)tb.Tag;

            try
            {
                le.IsValid1 = false;

                int ItemID = Convert.ToInt32(tb.Text);

                string name = "";
                string desc = "";
                if (Form1.THIS.GetItemInfo(ItemID, ref name, ref desc))
                {
                    le.rec.ItemID.v_int = ItemID;
                    le.IsValid1 = true;
                }
                else
                {
                    throw new ArgumentException("no such item");
                }

                tb.BackColor = Color.White;
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            le.lb2.Text = le.GetItemDesc();
        }

        private void OnTextChanged_ExpDays(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            LootEntry le = (LootEntry)tb.Tag;

            try
            {
                string[] gp = tb.Text.Split("-".ToCharArray());
                if (gp.Length > 1)
                {
                    le.rec.ExpDaysMin.v_int = Convert.ToInt32(gp[0]);
                    le.rec.ExpDaysMax.v_int = Convert.ToInt32(gp[1]);
                }
                else
                {
                    le.rec.ExpDaysMin.v_int = Convert.ToInt32(tb.Text);
                    le.rec.ExpDaysMax.v_int = Convert.ToInt32(tb.Text);
                }

                tb.BackColor = Color.White;
                RecalculateChances();
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            le.lb2.Text = le.GetItemDesc();
        }

        private void OnTextChanged_GDIfHave(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            LootEntry le = (LootEntry)tb.Tag;

            try
            {
                le.rec.GDIfHave.v_int = Convert.ToInt32(tb.Text);
                le.lb2.Text = le.GetItemDesc();
            }
            catch
            {
            }
        }

        private void OnTextChanged_GD(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            LootEntry le = (LootEntry)tb.Tag;

            try
            {
                le.IsValid1 = false;

                string[] gp = tb.Text.Split("-".ToCharArray());
                int gd1 = Convert.ToInt32(gp[0]);
                int gd2 = Convert.ToInt32(gp[1]);

                le.rec.GDMin.v_int = gd1;
                le.rec.GDMax.v_int = gd2;
                le.IsValid1 = true;

                tb.BackColor = Color.White;
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            le.lb2.Text = le.GetItemDesc();
        }

        private void OnTextChanged_Chance(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            LootEntry le = (LootEntry)tb.Tag;

            try
            {
                le.rec.Chance.v_double = Convert.ToDouble(tb.Text);

                tb.BackColor = Color.White;
                RecalculateChances();
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            return;
        }

        private void buttonDelete_Click(object sender, EventArgs e)
        {
            Button tb = (Button)sender;
            LootEntry le = (LootEntry)tb.Tag;

            loots_.Remove(le);
            RedrawUI();
        }

        private void buttonNewGD_Click(object sender, EventArgs e)
        {
            LootEntry le = new LootEntry(0);
            loots_.Add(le);

            RedrawUI();
        }

        private void buttonNewItem_Click(object sender, EventArgs e)
        {
            LootEntry le = new LootEntry(1);
            loots_.Add(le);

            RedrawUI();
        }

        private void buttonResort_Click(object sender, EventArgs e)
        {
            loots_.Sort(LootEntry.CompareByChanceDesc);
            RedrawUI();
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            SaveToDB();
        }
    }

    public class LootEntry
    {
        public int DropType = 0;
        public bool IsValid1 = false;

        public DBLootEntry rec;

        public TextBox ed1 = null;  // item id or GD
        public TextBox ed2 = null;  // chance
        public TextBox ed3 = null;  // exp days
        public TextBox ed4 = null;  // gd to refund if we already have that item
        public Label lb1 = null;
        public Label lb2 = null;
        public Label lb3 = null;

        public double nChance = 0;

        public static int CompareByChanceDesc(LootEntry x, LootEntry y)
        {
            if (x.rec.Chance.v_double < y.rec.Chance.v_double) return 1;
            else if (x.rec.Chance.v_double > y.rec.Chance.v_double) return -1;
            return 0;
        }

        public LootEntry(int in_DropType)
        {
            DropType = in_DropType;
            
            rec = new DBLootEntry();
            rec.SetNewRecordDefaults();
        }

        public LootEntry(DBLootEntry in_rec)
        {
            rec = in_rec;
            DropType = (rec.ItemID.AsInt() > 0) ? 1 : 0;
            IsValid1 = true;
        }

        public void DeleteUI()
        {
            if (ed1 != null) ed1.Dispose();
            if (ed2 != null) ed2.Dispose();
            if (ed3 != null) ed3.Dispose();
            if (ed4 != null) ed4.Dispose();
            if (lb1 != null) lb1.Dispose();
            if (lb2 != null) lb2.Dispose();
            if (lb3 != null) lb3.Dispose();
        }

        public string GetItemDesc()
        {
            if (DropType == 0 && !IsValid1)
                    return "Enter MinGD-MaxGD";
            if (DropType == 0 && IsValid1)
                return string.Format("GD: {0}-{1}", rec.GDMin.v_int, rec.GDMax.v_int);

            if (DropType == 1 && !IsValid1)
                    return "Enter ItemID";
            if (DropType == 1 && IsValid1)
            {
                string name = "";
                string desc = "";
                if (Form1.THIS.GetItemInfo(rec.ItemID.v_int, ref name, ref desc))
                {
                    return string.Format("{0} for {1}-{2} days, {3} GD If Have", name, rec.ExpDaysMin.v_int, rec.ExpDaysMax.v_int, rec.GDIfHave.v_int);
                }
                else
                {
                    return string.Format("***UNKNOWN*** item");
                }
            }

            return "<error>";
        }

    };

}
