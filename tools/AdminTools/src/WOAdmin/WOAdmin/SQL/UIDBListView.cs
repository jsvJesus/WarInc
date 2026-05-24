using System;
using System.Collections.Generic;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace WOAdmin
{
    abstract public class UIDBListView<T_DBTYPE> where T_DBTYPE : SQLRec, new()
    {
        public Dictionary<int, T_DBTYPE> data_ = new Dictionary<int, T_DBTYPE>();

        public UIDBListView()
        {
        }

        public T_DBTYPE SearchByKey(int key)
        {
            T_DBTYPE value;
            if (data_.TryGetValue(key, out value) == true)
                return value;
            return null;
        }

        abstract protected string GET_TABLE_NAME();

        public void ReadFromDB()
        {
            data_.Clear();

            string table = GET_TABLE_NAME();

            System.Data.SqlClient.SqlDataReader reader;
            Form1.sql.SelectAll(table, out reader);
            if (reader == null)
                return;

            while (reader.Read())
            {
                SQLRec rec = new T_DBTYPE(); // CREATE_REC_INSTANCE();
                rec.Fill(reader);

                data_.Add(rec.RECORD_ID.v_int, rec as T_DBTYPE);
            }
            reader.Close();
        }

        // strange function, used only after adding new record to DB...
        public void TEMP_ScanForNewInDB()
        {
            string table = GET_TABLE_NAME();

            System.Data.SqlClient.SqlDataReader reader;
            Form1.sql.SelectAll(table, out reader);
            if (reader == null)
                return;

            ListViewItem haveNew = null;

            while (reader.Read())
            {
                SQLRec rec = new T_DBTYPE(); // CREATE_REC_INSTANCE();
                rec.Fill(reader);

                if (data_.ContainsKey(rec.RECORD_ID.AsInt()) == true)
                    continue;

                data_.Add(rec.RECORD_ID.v_int, rec as T_DBTYPE);
                ListViewItem lvi = AddToListView(rec);

                if (haveNew == null)
                {
                    haveNew = lvi;
                }
            }
            reader.Close();

            if (haveNew != null)
            {
                listView.SelectedIndices.Clear();
                haveNew.Selected = true;
                haveNew.EnsureVisible();
            }
        }

        //
        //
        // list view part...
        //
        //

        // attached listview.
        public ListView listView = null;

        public void AttachToListView(ListView in_listView)
        {
            Debug.Assert(listView == null);
            listView = in_listView;
            CreateListViewColumns();
        }

        abstract protected void CreateListViewColumns();
        abstract public void UpdateListView(SQLRec rec);

        List<ListViewItem> addColl_ = null;

        public void BeginListViewUpdate()
        {
            if (addColl_ != null) MessageBox.Show("BeginListViewUpdate:: addColl_");

            listView.BeginUpdate();
            listView.SelectedIndices.Clear();
            listView.Items.Clear();

            addColl_ = new List<ListViewItem>();
        }

        public void EndListViewUpdate()
        {
            if (addColl_ == null) MessageBox.Show("EndListViewUpdate:: addColl_");

            if (listView.Items.Count > 0)
                listView.SelectedIndices.Add(0);

            listView.Items.AddRange(addColl_.ToArray());
            listView.EndUpdate();

            addColl_ = null;
        }

        public ListViewItem AddToListView(SQLRec rec)
        {
            ListViewItem lvi = new ListViewItem();
            lvi.Tag = rec;
            rec.ui_lvi = lvi;
            UpdateListView(rec);

            if (addColl_ == null)
            {
                // if adding directly without batch
                listView.Items.Add(lvi);
            }
            else
            {
                addColl_.Add(lvi);
            }
            return lvi;
        }
    }
}
