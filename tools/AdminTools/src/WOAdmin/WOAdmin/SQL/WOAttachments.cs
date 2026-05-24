using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public class WOAttachments : UIDBListView<DBAttachment>
    {
        public WOAttachments()
        {
        }

        override protected string GET_TABLE_NAME()
        {
            return DBAttachment.GET_TABLE_NAME();
        }

        override protected void CreateListViewColumns()
        {
            listView.Columns.Clear();
            listView.Columns.Add("Name", 120);
            listView.Columns.Add("FNAME", 120);
            listView.Columns.Add("Type", 100);
            listView.Columns.Add("SpecID", 100);

            //shop
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
            DBAttachment rec = in_rec as DBAttachment;
            System.Diagnostics.Debug.Assert(rec != null);
            ListViewItem lvi = rec.ui_lvi;
            System.Diagnostics.Debug.Assert(lvi != null);

            lvi.SubItems.Clear();
            lvi.UseItemStyleForSubItems = false;
            lvi.Text = rec.Name.ToString();
            lvi.SubItems.Add(rec.FNAME.ToString());
            lvi.SubItems.Add(DBAttachment.TypeValues.GetNameByType(rec.Type.ToString()));
            lvi.SubItems.Add(rec.SpecID.ToString());

            //shop
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
