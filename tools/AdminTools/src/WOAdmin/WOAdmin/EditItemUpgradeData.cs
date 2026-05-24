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
    public partial class EditItemUpgradeData : Form
    {
        int[] cbItemType = new int[1000];
        DBWeapon[] cbWeaponsMap = new DBWeapon[1000];
        int SelectedCategory = 0;
        int SelectedUpgrade = 0;
        DBWeapon SelectedWeapon = null;

        ///public enum EUpgradeType
        public const int UP_Damage = 0;
        public const int UP_Spread = 1;
        public const int UP_Recoil = 2;
        public const int UP_Firerate = 3;
        public const int UP_ClipSize = 4;
        public const int UP_MAX_UPGRADES = 5;
        bool IsUpgradeAbsolute(int type)
        {
            return type == UP_ClipSize || type == UP_Firerate;
        }

        static public string[] upgradeNames = new string[UP_MAX_UPGRADES] {
            "Damage",
            "Spread",
            "Recoil",
            "Firerate",
            "ClipSize"
        };

        Label lblValueClipSize;
        Label lblValueOthers;

        const int MAX_UPGRADE_LEVEL = 10;

        UpgradeData[] table_ = new UpgradeData[MAX_UPGRADE_LEVEL];

        public EditItemUpgradeData(int in_BoxID, string in_name)
        {
            InitializeComponent();

            CreateUI();

            foreach (EditField_Combo.SPair pair in DBWeapon.CategoryValues)
            {
                int idx = cbWeapType.Items.Add(pair.name);
                cbItemType[idx] = Convert.ToInt32(pair.type);
            }

            for (int i = 0; i < UP_MAX_UPGRADES; i++)
            {
                cbUpgradeId.Items.Add(upgradeNames[i]);
            }
            cbUpgradeId.SelectedIndex = 0;
        }

        void LoadFromDB()
        {
            for (int level = 0; level < MAX_UPGRADE_LEVEL; level++)
            {
                table_[level].Reset();
            }


            SqlCommand sqcmd = new SqlCommand();
            sqcmd.CommandText = string.Format("select * from {0} where Category={1} and UpgradeID={2}",
                DBItemUpgradeData.GET_TABLE_NAME(),
                SelectedCategory,
                SelectedUpgrade);

            SqlDataReader reader;
            Form1.sql.Select(sqcmd, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                DBItemUpgradeData rec = new DBItemUpgradeData();
                rec.Fill(reader);

                // ItemID in form of 6CCULL
                int ItemID = rec.ItemID.AsInt();
                if (ItemID < 600000 || ItemID >= 700000)
                    throw new ArgumentException("Invalid Upgrade ItemID: " + ItemID);
                int Category = Convert.ToInt32(ItemID.ToString().Substring(1, 2));
                int UpgradeID = Convert.ToInt32(ItemID.ToString().Substring(3, 1));
                int UpgradeLevel = Convert.ToInt32(ItemID.ToString().Substring(4, 2));

                if(UpgradeLevel >= MAX_UPGRADE_LEVEL)
                    throw new ArgumentException("Invalid UpgradeLevel ItemID: " + ItemID);

                if(UpgradeID >= UP_MAX_UPGRADES)
                    throw new ArgumentException("Invalid UpgradeID ItemID: " + ItemID);

                table_[UpgradeLevel].FromRec(rec);
            }
            reader.Close();

            RecalculateChances();
        }

        void SaveToDB()
        {
            string cmd = string.Format("delete from {0} where Category={1} and UpgradeID={2}",
                DBItemUpgradeData.GET_TABLE_NAME(),
                SelectedCategory,
                SelectedUpgrade);
            Form1.sql.Exec(cmd, false);

            for (int level = 0; level < MAX_UPGRADE_LEVEL; level++)
            {
                UpgradeData ud = table_[level];
                if (!ud.IsValid)
                    continue;

                DBItemUpgradeData rec = new DBItemUpgradeData();
                ud.ToRec(SelectedCategory, SelectedUpgrade, level, rec);
                Form1.sql.InsertNew(rec);
            }
        }

        void RecalculateChances()
        {
            double curChanceGP = 1;
            double curChanceGD = 1;
            double curPriceGP = 0;
            double curPriceGD = 0;

            int level = 0;
            int disabledLevel = 100;
            for (; level < MAX_UPGRADE_LEVEL; level++)
            {
                UpgradeData ud = table_[level];

                curChanceGP *= (ud.GPChance / 100.0);
                curChanceGD *= (ud.GDChance / 100.0);

                if(curChanceGP > 0)
                    curPriceGP += (ud.GPPrice + (ud.GPPrice * (1.0 / curChanceGP))) / 2;
                if(curChanceGD > 0)
                    curPriceGD += (ud.GDPrice + (ud.GDPrice * (1.0 / curChanceGD))) / 2;

                ud.CurChanceGP = curChanceGP;
                ud.CurChanceGD = curChanceGD;
                ud.CurPriceGD = curPriceGD;
                ud.CurPriceGP = curPriceGP;
                ud.IsValid = false;

                if (level >= disabledLevel)
                {
                    break;
                }

                ud.SetUIVisibility(true);
                ud.SetCurChances();
                ud.SetUpgradedValue(SelectedWeapon, SelectedUpgrade);

                if (disabledLevel >= 100 && ud.GDPrice < 0.001f && ud.GPPrice < 0.001f)
                {
                    ud.HideChances();
                    disabledLevel = level + 1;
                    continue;
                }

                ud.IsValid = true;
            }

            for(; level < MAX_UPGRADE_LEVEL; level++)
            {
                table_[level].SetUIVisibility(false);
            }
        }


        TextBox CreateEditBox(ref int x, int y, int width, UpgradeData ud, int mode)
        {
            TextBox tb = new TextBox();
            tb.Location = new System.Drawing.Point(x, y);
            tb.Size = new System.Drawing.Size(width, 23);
            //tb.BackColor = System.Drawing.SystemColors.Control;
            tb.Text = "0";
            if(mode == 0)
                tb.TextChanged += new System.EventHandler(this.OnTextChanged_Value);
            else if(mode == 1)
                tb.TextChanged += new System.EventHandler(this.OnTextChanged_GD);
            else if (mode == 2)
                tb.TextChanged += new System.EventHandler(this.OnTextChanged_Chance);

            tb.Tag = ud;
            panel1.Controls.Add(tb);

            x += width + 10;

            return tb;
        }

        Label CreateLabel(ref int x, int y, int width)
        {
            Label lbl = new Label();
            lbl.Location = new System.Drawing.Point(x, y);
            lbl.Size = new System.Drawing.Size(width, 20);
            lbl.Text = "xxxxxxxxxxxx";
            panel1.Controls.Add(lbl);

            x += width + 10;

            return lbl;
        }

        void CreateUI()
        {
            int x = 0;
            int y = 0;

            //panel1.Controls.Clear();

            int[] widths = { 20, 80, 80, 80, 80, 80, 120 };
            string[] descs = { "LV", "GC Cost", "GC Chance", "WI Cost", "WI Chance", "X", "X" };

            for (int i = 0; i < 5; i++)
            {
                Label lbl = new Label();
                lbl.Location = new System.Drawing.Point(x, y);
                lbl.Size = new System.Drawing.Size(widths[i], 23);
                lbl.Text = descs[i];
                lbl.ForeColor = Color.Blue;
                panel1.Controls.Add(lbl);
                x += widths[i] + 10;
            }
            
            // switch between % or value
            {
                Label lbl = new Label();
                lbl.Location = new System.Drawing.Point(x, y);
                lbl.Size = new System.Drawing.Size(widths[5], 23);
                lbl.Text = "Value (+Fixed)";
                lbl.ForeColor = Color.Blue;
                panel1.Controls.Add(lbl);
                lblValueClipSize = lbl;
                lblValueClipSize.Hide();

                lbl = new Label();
                lbl.Location = new System.Drawing.Point(x, y);
                lbl.Size = new System.Drawing.Size(widths[5], 23);
                lbl.Text = "Value (+ %)";
                lbl.ForeColor = Color.Blue;
                panel1.Controls.Add(lbl);
                lblValueOthers = lbl;
                x += widths[5] + 10;
            }

            y += 25;

            // recreate UI elements
            for(int level=0; level<MAX_UPGRADE_LEVEL; level++)
            {
                x = 0;

                Label lbl = new Label();
                lbl.Location = new System.Drawing.Point(x, y);
                lbl.Size = new System.Drawing.Size(widths[0], 23);
                lbl.Text = (level + 1).ToString();
                panel1.Controls.Add(lbl);
                x += widths[0] + 10;

                table_[level] = new UpgradeData();
                UpgradeData ud = table_[level];

                // create edit boxes for price/chance value
                ud.ed1 = CreateEditBox(ref x, y, widths[1], ud, 1);
                ud.ed2 = CreateEditBox(ref x, y, widths[2], ud, 2);
                ud.ed3 = CreateEditBox(ref x, y, widths[3], ud, 1);
                ud.ed4 = CreateEditBox(ref x, y, widths[4], ud, 2);
                ud.ed5 = CreateEditBox(ref x, y, widths[5], ud, 0);

                // create labels for chance display
                x = widths[0] + 10;
                y += 25;
                ud.lb1 = CreateLabel(ref x, y, widths[1]);
                ud.lb2 = CreateLabel(ref x, y, widths[2]);
                ud.lb3 = CreateLabel(ref x, y, widths[3]);
                ud.lb4 = CreateLabel(ref x, y, widths[4]);
                ud.lb5 = CreateLabel(ref x, y, widths[5]);

                y += 20;
            }

            RecalculateChances();
        }

        private void OnTextChanged_GD(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            UpgradeData ud = (UpgradeData)tb.Tag;

            if (ud.insideLocalSetText)
                return;

            try
            {
                if (tb == ud.ed1) ud.GPPrice = Convert.ToInt32(tb.Text);
                if (tb == ud.ed3) ud.GDPrice = Convert.ToInt32(tb.Text);
                tb.BackColor = Color.White;
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            RecalculateChances();
        }

        private void OnTextChanged_Chance(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            UpgradeData ud = (UpgradeData)tb.Tag;

            if (ud.insideLocalSetText)
                return;

            try
            {
                if (tb == ud.ed2) ud.GPChance = Convert.ToDouble(tb.Text);
                if (tb == ud.ed4) ud.GDChance = Convert.ToDouble(tb.Text);
                tb.BackColor = Color.White;
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            RecalculateChances();
            return;
        }

        private void OnTextChanged_Value(object sender, EventArgs e)
        {
            TextBox tb = (TextBox)sender;
            UpgradeData ud = (UpgradeData)tb.Tag;

            if (ud.insideLocalSetText)
                return;

            try
            {
                ud.Value = Convert.ToDouble(tb.Text);
                tb.BackColor = Color.White;
            }
            catch
            {
                tb.BackColor = Color.Red;
            }

            RecalculateChances();
            return;
        }

        private void EditItemUpgradeData_Load(object sender, EventArgs e)
        {

        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            SaveToDB();
        }

        private void cbWeapType_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedCategory = cbItemType[cbWeapType.SelectedIndex];

            LoadFromDB();

            // repopulate weapon list
            cbSelectWeapon.Items.Clear();
            foreach (DBWeapon wpn in Form1.weapons_.data_.Values)
            {
                if (wpn.Category.AsInt() != SelectedCategory)
                    continue;
                int idx = cbSelectWeapon.Items.Add(wpn.Name.ToString());
                cbWeaponsMap[idx] = wpn;
            }
            cbSelectWeapon.SelectedIndex = 0;
        }

        private void cbUpgradeId_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedUpgrade = cbUpgradeId.SelectedIndex;
            LoadFromDB();

            if (IsUpgradeAbsolute(SelectedUpgrade))
            {
                lblValueClipSize.Show();
                lblValueOthers.Hide();
            }
            else
            {
                lblValueClipSize.Hide();
                lblValueOthers.Show();
            }
        }

        private void cbSelectWeapon_SelectedIndexChanged(object sender, EventArgs e)
        {
            SelectedWeapon = cbWeaponsMap[cbSelectWeapon.SelectedIndex];
            RecalculateChances();
        }


        private void EditItemUpgradeData_Shown(object sender, EventArgs e)
        {
            cbWeapType.SelectedIndex = 0;
        }

    }

    public class UpgradeData
    {
        public bool IsValid = false;

        // actual data
        public int GPPrice = 0;
        public double GPChance = 0;
        public int GDPrice = 0;
        public double GDChance = 0;
        public double Value = 0;

        // recalculated vars for UI
        public double CurChanceGP = 0;
        public double CurChanceGD = 0;
        public double CurPriceGP = 0;
        public double CurPriceGD = 0;

        public bool insideLocalSetText = false;
        public TextBox ed1 = null;  // GPCost
        public TextBox ed2 = null;  // GPChance
        public TextBox ed3 = null;  // GDCost
        public TextBox ed4 = null;  // GDChance
        public TextBox ed5 = null;  // Value

        public Label lb1 = null;
        public Label lb2 = null;
        public Label lb3 = null;
        public Label lb4 = null;
        public Label lb5 = null;

        public void Reset()
        {
            GPPrice = 0;
            GPChance = 0;
            GDPrice = 0;
            GDChance = 0;
            Value = 0;

            SetEditTexts();
        }

        public void FromRec(DBItemUpgradeData rec)
        {
            GPPrice = rec.GPPrice.v_int;
            GPChance = rec.GPChance.v_double;
            GDPrice = rec.GDPrice.v_int;
            GDChance = rec.GDChance.v_double;
            Value = rec.Value.v_double;

            SetEditTexts();
        }

        void SetEditTexts()
        {
            insideLocalSetText = true;
            ed1.Text = GPPrice.ToString();
            ed2.Text = GPChance.ToString();
            ed3.Text = GDPrice.ToString();
            ed4.Text = GDChance.ToString();
            ed5.Text = Value.ToString();
            insideLocalSetText = false;
        }

        public void ToRec(int Category, int UpgradeId, int Level, DBItemUpgradeData rec)
        {
            // ItemID in form of 6CCULL
            int ItemID = 600000 + Category * 1000 + UpgradeId * 100 + Level;
            rec.ItemID.FromObject(ItemID);

            rec.Category.FromObject(Category);
            rec.UpgradeID.FromObject(UpgradeId);
            rec.GPPrice.FromObject(GPPrice);
            rec.GPChance.FromObject(GPChance);
            rec.GDPrice.FromObject(GDPrice);
            rec.GDChance.FromObject(GDChance);
            rec.Value.FromObject(Value);
            rec.Name.FromObject(string.Format("ItemUpgrade {0}-{1}-{2}", Category, UpgradeId, Level));
            rec.Description.FromObject(string.Format("{0} {1} Upgrade Lv {2}", 
                DBWeapon.CategoryValues.GetNameByType(Category.ToString()), 
                EditItemUpgradeData.upgradeNames[UpgradeId], 
                Level));
        }

        public void SetUIVisibility(bool enable)
        {
            ed1.Enabled = enable;
            ed2.Enabled = enable;
            ed3.Enabled = enable;
            ed4.Enabled = enable;
            ed5.Enabled = enable;
            lb1.Visible = enable;
            lb2.Visible = enable;
            lb3.Visible = enable;
            lb4.Visible = enable;
            lb5.Visible = enable;
        }

        public void HideChances()
        {
            lb1.Visible = false;
            lb2.Visible = false;
            lb3.Visible = false;
            lb4.Visible = false;
            lb5.Visible = false;
        }

        public void SetCurChances()
        {
            lb1.Text = string.Format("~ {0:.} GC", CurPriceGP);
            lb2.Text = string.Format("{0} %", CurChanceGP * 100);
            lb3.Text = string.Format("~ {0:.} WI", CurPriceGD);
            lb4.Text = string.Format("{0} %", CurChanceGD * 100);
        }

        public void SetUpgradedValue(DBWeapon wpn, int UpgradeType)
        {
            if (wpn == null)
                return;

            double val1 = 0;
            double val2 = 0;
            switch (UpgradeType)
            {
                case EditItemUpgradeData.UP_Damage:
                    val1 = wpn.Damage.v_double;
                    val2 = val1 + (val1 * (Value / 100));
                    break;
                case EditItemUpgradeData.UP_Spread:
                    val1 = wpn.Spread.v_double;
                    val2 = val1 + (val1 * (Value / 100));
                    break;
                case EditItemUpgradeData.UP_Recoil:
                    val1 = wpn.Recoil.v_double;
                    val2 = val1 + (val1 * (Value / 100));
                    break;
                case EditItemUpgradeData.UP_Firerate:
                    val1 = wpn.RateOfFire.v_int;
                    val2 = val1 + Value;
                    break;
                case EditItemUpgradeData.UP_ClipSize:
                    val1 = wpn.Clipsize.v_int;
                    val2 = val1 + Value;
                    break;
            }

            lb5.Text = string.Format("{0:0.0}->{1:0.0}", val1, val2);
        }

    };

}
