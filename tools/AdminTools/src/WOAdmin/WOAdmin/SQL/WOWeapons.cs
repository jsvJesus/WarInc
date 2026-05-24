using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public class WOWeapons : UIDBListView<DBWeapon>
    {
        public WOWeapons()
        {
        }

        override protected string GET_TABLE_NAME()
        {
            return DBWeapon.GET_TABLE_NAME();
        }

        override protected void CreateListViewColumns()
        {
            listView.Columns.Clear();
            listView.Columns.Add("Name", 120);
            listView.Columns.Add("FNAME", 120);
            listView.Columns.Add("Damage", 50);
            listView.Columns.Add("Recoil", 50);
            listView.Columns.Add("Spread", 50);
            listView.Columns.Add("DamageDecay", 50);
            listView.Columns.Add("Clipsize", 50);
            listView.Columns.Add("NumClips", 50);
            listView.Columns.Add("RateOfFire", 50);
            listView.Columns.Add("LevelRequired", 50);
            listView.Columns.Add("New", 40);
            listView.Columns.Add("UP", 40);
            listView.Columns.Add("FPS", 40);
            listView.Columns.Add("1 day", 50);
            listView.Columns.Add("7 days", 50);
            listView.Columns.Add("30 days", 50);
            listView.Columns.Add("Permanent", 50);
            listView.Columns.Add("GD 1", 50);
            listView.Columns.Add("GD 7", 50);
            listView.Columns.Add("GD 30", 50);
            listView.Columns.Add("GD Perm", 50);
            listView.Columns.Add("Description", 1024);
        }

        override public void UpdateListView(SQLRec in_rec)
        {
            DBWeapon rec = in_rec as DBWeapon;
            System.Diagnostics.Debug.Assert(rec != null);
            ListViewItem lvi = rec.ui_lvi;
            System.Diagnostics.Debug.Assert(lvi != null);

            if(rec.DamageDecay.v_double <=0)
            {
                MessageBox.Show("DamageDecay cannot be 0 or negative! Correct it!");
            }

            lvi.SubItems.Clear();
            lvi.UseItemStyleForSubItems = false;
            lvi.Text = rec.Name.ToString();
            lvi.SubItems.Add(rec.FNAME.ToString());
            lvi.SubItems.Add(rec.Damage.ToString());
            lvi.SubItems.Add(rec.Recoil.ToString());
            lvi.SubItems.Add(rec.Spread.ToString());
            lvi.SubItems.Add(rec.DamageDecay.ToString());
            lvi.SubItems.Add(rec.Clipsize.ToString());
            lvi.SubItems.Add(rec.NumClips.ToString());
            lvi.SubItems.Add(rec.RateOfFire.ToString());
            lvi.SubItems.Add(rec.LevelRequired.ToString());
            lvi.SubItems.Add(rec.IsNew.AsInt() == 0 ? "" : "NEW");
            lvi.SubItems.Add(rec.IsUpgradeable.AsInt() == 0 ? " " : "+");
            lvi.SubItems.Add(rec.IsFPS.AsInt() == 0 ? " " : "+");
            lvi.SubItems.Add(rec.Price1.ToString()).BackColor = WOItems.priceBgClr;
            lvi.SubItems.Add(rec.Price7.ToString()).BackColor = WOItems.priceBgClr;
            lvi.SubItems.Add(rec.Price30.ToString()).BackColor = WOItems.priceBgClr;
            lvi.SubItems.Add(rec.PriceP.ToString()).BackColor = WOItems.priceBgClr;
            lvi.SubItems.Add(rec.GPrice1.ToString()).BackColor = WOItems.priceBgClr2;
            lvi.SubItems.Add(rec.GPrice7.ToString()).BackColor = WOItems.priceBgClr2;
            lvi.SubItems.Add(rec.GPrice30.ToString()).BackColor = WOItems.priceBgClr2;
            lvi.SubItems.Add(rec.GPriceP.ToString()).BackColor = WOItems.priceBgClr2;
            lvi.SubItems.Add(rec.Description.ToString());
        }
    }
}
