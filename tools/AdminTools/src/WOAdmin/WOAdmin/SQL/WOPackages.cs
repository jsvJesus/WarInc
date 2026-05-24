using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public class WOPackages : UIDBListView<DBPackage>
    {
        public WOPackages()
        {
        }

        override protected string GET_TABLE_NAME()
        {
            return DBPackage.GET_TABLE_NAME();
        }

        override protected void CreateListViewColumns()
        {
            listView.Columns.Clear();
            listView.Columns.Add("Name", 120);
            listView.Columns.Add("FNAME", 120);
            listView.Columns.Add("IsEnabled", 50);
            listView.Columns.Add("LevelRequired", 50);
            listView.Columns.Add("New", 40);
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
            ListViewItem lvi = in_rec.ui_lvi;
            System.Diagnostics.Debug.Assert(lvi != null);
            DBPackage rec = in_rec as DBPackage;
            System.Diagnostics.Debug.Assert(rec != null);

            lvi.SubItems.Clear();
            lvi.UseItemStyleForSubItems = false;
            lvi.Text = rec.Name.ToString();
            lvi.SubItems.Add(rec.FNAME.ToString());
            lvi.SubItems.Add(rec.IsEnabled.ToString());
            lvi.SubItems.Add(rec.LevelRequired.ToString());
            lvi.SubItems.Add(rec.IsNew.AsInt() == 0 ? "" : "NEW");
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
