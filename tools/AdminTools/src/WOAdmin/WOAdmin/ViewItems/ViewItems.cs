using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.Diagnostics;

namespace WOAdmin
{
    public partial class ViewItems : Form, IAdminView
    {
        int[] cbItemType = new int[1000];

        FormEditWeapon editW_;
        FormEditGear editG_;
        FormEditItem editI_;
        FormEditAttachment editA_;
        FormEditPackage editP_;
        FormEditUsableItem editU_;
        FormEditSKU editSku_;

        Button btnLootBox_;
        Label lblLootBox_;

        ListView curListView_ = null;

        public ViewItems()
        {
            InitializeComponent();

            // place all listviews in same position
            listViewW.Hide();
            listViewG.Hide();
            listViewI.Hide();
            listViewP.Hide();
            listViewA.Hide();
            listViewW.Dock = DockStyle.Fill;
            listViewG.Dock = DockStyle.Fill;
            listViewI.Dock = DockStyle.Fill;
            listViewP.Dock = DockStyle.Fill;
            listViewA.Dock = DockStyle.Fill;
            listViewW.HideSelection = false;
            listViewG.HideSelection = false;
            listViewI.HideSelection = false;
            listViewP.HideSelection = false;
            listViewA.HideSelection = false;

            PlaceEditForms();

            Form1.weapons_.AttachToListView(listViewW);
            Form1.gears_.AttachToListView(listViewG);
            Form1.items_.AttachToListView(listViewI);
            Form1.packages_.AttachToListView(listViewP);
            Form1.attachments_.AttachToListView(listViewA);

            listViewW.ListViewItemSorter = new ListViewColumnSorter();
            listViewW.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
            listViewG.ListViewItemSorter = new ListViewColumnSorter();
            listViewG.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
            listViewI.ListViewItemSorter = new ListViewColumnSorter();
            listViewI.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);
            listViewA.ListViewItemSorter = new ListViewColumnSorter();
            listViewA.ColumnClick += new System.Windows.Forms.ColumnClickEventHandler(ListViewColumnSorter.listView1_ColumnClick);

            cbWeapType.Items.Add("= Select Category =");
            foreach (EditField_Combo.SPair pair in DBWeapon.CategoryValues)
            {
                int idx = cbWeapType.Items.Add(pair.name);
                cbItemType[idx] = Convert.ToInt32(pair.type);
            }
            foreach (EditField_Combo.SPair pair in DBGear.CategoryValues)
            {
                int idx = cbWeapType.Items.Add(pair.name);
                cbItemType[idx] = Convert.ToInt32(pair.type);
            }
            foreach (EditField_Combo.SPair pair in DBItem.CategoryValues)
            {
                int idx = cbWeapType.Items.Add(pair.name);
                cbItemType[idx] = Convert.ToInt32(pair.type);
            }
            foreach (EditField_Combo.SPair pair in DBPackage.CategoryValues)
            {
                int idx = cbWeapType.Items.Add(pair.name);
                cbItemType[idx] = Convert.ToInt32(pair.type);
            }
            foreach (EditField_Combo.SPair pair in DBAttachment.CategoryValues)
            {
                int idx = cbWeapType.Items.Add(pair.name);
                cbItemType[idx] = Convert.ToInt32(pair.type);
            }
            cbWeapType.SelectedIndex = 0;
        }

        void InitEditForm(EditRecord edit, System.EventHandler saveHandler)
        {
            edit.TopLevel = false;
            edit.Parent = splitContainer2.Panel1;
            edit.FormBorderStyle = FormBorderStyle.None;
            edit.Dock = DockStyle.Fill;
            edit.DisableAll();
            edit.AttachSaveButton(saveHandler);
            edit.AttachDeleteButton(btnDelete_Click);
            edit.Hide();
        }

        void CheckFPSAttach(ref string msg, DBWeapon wpn, int slot, int AttachID, int SpecID)
        {
            // check for slot in default attachment
            if(AttachID > 0)
            {
                DBAttachment att = Form1.attachments_.SearchByKey(AttachID);
                if (att == null)
                {
                    msg += string.Format("{0}:{1} : FPSAttach{2} ItemID:{3} not exist\n",
                        wpn.RECORD_ID.AsInt(), wpn.Name.ToString(), slot, AttachID);
                }
                else if (att.Type.AsInt() != slot)
                {
                    msg += string.Format("{0}:{1} : FPSAttach{2} ItemID:{3} slot mismatch {4}\n",
                        wpn.RECORD_ID.AsInt(), wpn.Name.ToString(), slot, AttachID, att.Type.AsInt());
                }
            }

            // check for items with SpecIDs
            if (SpecID == 0)
                return;

            int found = 0;
            foreach (DBAttachment att in Form1.attachments_.data_.Values)
            {
                if (att.SpecID.AsInt() != SpecID)
                    continue;

                found++;
                if (att.Type.AsInt() != slot)
                {
                    msg += string.Format("{0}:{1} : FPSSpec{2} ItemID:{3}:{4} slot mismatch {5}\n",
                        wpn.RECORD_ID.AsInt(), wpn.Name.ToString(), slot, att.RECORD_ID.AsInt(), att.Name.ToString(), att.Type.AsInt());
                }
            }
            if (found == 0)
            {
                msg += string.Format("{0}:{1} : FPSSpec{2} - no matching items\n",
                    wpn.RECORD_ID.AsInt(), wpn.Name.ToString(), slot);
            }
        }

        void TEMP_ValidateFPSAttachments()
        {
            string msg = "";
            foreach (DBWeapon wpn in Form1.weapons_.data_.Values)
            {
                CheckFPSAttach(ref msg, wpn, 0, wpn.FPSAttach0.AsInt(), wpn.FPSSpec0.AsInt());
                CheckFPSAttach(ref msg, wpn, 1, wpn.FPSAttach1.AsInt(), wpn.FPSSpec1.AsInt());
                CheckFPSAttach(ref msg, wpn, 2, wpn.FPSAttach2.AsInt(), wpn.FPSSpec2.AsInt());
                CheckFPSAttach(ref msg, wpn, 3, wpn.FPSAttach3.AsInt(), wpn.FPSSpec3.AsInt());
                CheckFPSAttach(ref msg, wpn, 4, wpn.FPSAttach4.AsInt(), wpn.FPSSpec4.AsInt());
                CheckFPSAttach(ref msg, wpn, 5, wpn.FPSAttach5.AsInt(), wpn.FPSSpec5.AsInt());
                CheckFPSAttach(ref msg, wpn, 6, wpn.FPSAttach6.AsInt(), wpn.FPSSpec6.AsInt());
                CheckFPSAttach(ref msg, wpn, 7, wpn.FPSAttach7.AsInt(), wpn.FPSSpec7.AsInt());
                CheckFPSAttach(ref msg, wpn, 8, wpn.FPSAttach8.AsInt(), wpn.FPSSpec8.AsInt());
            }

            if(msg.Length > 0)
                MessageBox.Show(msg);
        }

        void TEMP_AddLootDropEditButton()
        {
            Button btnSave_ = new Button();
            btnSave_ = new System.Windows.Forms.Button();
            btnSave_.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            btnSave_.BackColor = System.Drawing.SystemColors.ControlLight;
            btnSave_.Location = new System.Drawing.Point(editI_.Width - 100, 80);
            btnSave_.Name = "btnSave";
            btnSave_.Size = new System.Drawing.Size(75, 23);
            btnSave_.TabIndex = 15;
            btnSave_.Text = "EDIT LOOT";
            btnSave_.UseVisualStyleBackColor = false;
            btnSave_.Click += btnEditLootBox_Click;
            editI_.Controls.Add(btnSave_);

            btnLootBox_ = btnSave_;
            btnLootBox_.Hide();

            lblLootBox_ = new Label();
            lblLootBox_.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            lblLootBox_.Location = new System.Drawing.Point(editI_.Width - 100, 110);
            lblLootBox_.Size = new System.Drawing.Size(75, 23);
            lblLootBox_.Text = "";
            lblLootBox_.Hide();
            editI_.Controls.Add(lblLootBox_);
        }

        void PlaceEditForms()
        {
            editW_ = new FormEditWeapon();
            InitEditForm(editW_, btnSaveW_Click);

            editU_ = new FormEditUsableItem();
            InitEditForm(editU_, btnSaveW_Click); // usable item is actually a weapon

            editG_ = new FormEditGear();
            InitEditForm(editG_, btnSaveG_Click);

            editI_ = new FormEditItem();
            InitEditForm(editI_, btnSaveI_Click);

            editP_ = new FormEditPackage();
            InitEditForm(editP_, btnSaveP_Click);

            editA_ = new FormEditAttachment();
            InitEditForm(editA_, btnSaveA_Click);

            TEMP_AddLootDropEditButton();

            editSku_ = new FormEditSKU();
            editSku_.TopLevel = false;
            editSku_.Parent = splitContainer2.Panel2;
            editSku_.FormBorderStyle = FormBorderStyle.None;
            editSku_.Dock = DockStyle.Fill;
            editSku_.AttachSaveButton();
            editSku_.AttachDeleteButton();
            editSku_.DisableAll();
            editSku_.Show();
        }

        void IAdminView.OnViewActivated()
        {
            Form1.weapons_.ReadFromDB();
            Form1.gears_.ReadFromDB();
            Form1.items_.ReadFromDB();
            Form1.packages_.ReadFromDB();
            Form1.attachments_.ReadFromDB();

            if (cbWeapType.SelectedIndex == 0)
            {
                // select first category by default. listview will update itself here
                cbWeapType.SelectedIndex = 1;

                TEMP_ValidateFPSAttachments();
            }
        }

        void IAdminView.OnViewDeactivated()
        {
        }

        private void btnEditLootBox_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            DBItem rec = (DBItem)er.sqlrec_;
            if (rec == null)
                return;

            EditLootDrop ddd = new EditLootDrop(rec.RECORD_ID.AsInt(), rec.Name.ToString());
            ddd.Show();
        }

        private void btnDelete_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            int itemId = er.sqlrec_.RECORD_ID.AsInt();
            if (!er.ERDelete())
                return;
        }

        private void btnSaveW_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.weapons_.UpdateListView(er.sqlrec_);
        }

        private void btnSaveG_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.gears_.UpdateListView(er.sqlrec_);
        }

        private void btnSaveI_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.items_.UpdateListView(er.sqlrec_);
        }

        private void btnSaveP_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.packages_.UpdateListView(er.sqlrec_);
        }

        private void btnSaveA_Click(object sender, EventArgs e)
        {
            EditRecord er = (EditRecord)((Control)sender).Parent;
            er.ERSave();
            Form1.attachments_.UpdateListView(er.sqlrec_);
        }

        void PopulateWeaponsView(int filter)
        {
            Form1.weapons_.BeginListViewUpdate();

            foreach (DBWeapon rec in Form1.weapons_.data_.Values)
            {
                if (filter != 0 && rec.Category.v_int != filter)
                    continue;
                Form1.weapons_.AddToListView(rec);
            }

            Form1.weapons_.EndListViewUpdate();
        }

        void PopulateGearsView(int filter)
        {
            Form1.gears_.BeginListViewUpdate();

            foreach (DBGear rec in Form1.gears_.data_.Values)
            {
                if (filter != 0 && rec.Category.v_int != filter)
                    continue;

                Form1.gears_.AddToListView(rec);
            }

            Form1.gears_.EndListViewUpdate();
        }

        void PopulateItemsView(int filter)
        {
            Form1.items_.BeginListViewUpdate();

            foreach (DBItem rec in Form1.items_.data_.Values)
            {
                if (filter != 0 && rec.Category.v_int != filter)
                    continue;

                Form1.items_.AddToListView(rec);
            }

            Form1.items_.EndListViewUpdate();
        }

        void PopulatePackagesView(int filter)
        {
            Form1.packages_.BeginListViewUpdate();

            foreach (DBPackage rec in Form1.packages_.data_.Values)
            {
                if (filter != 0 && rec.Category.v_int != filter)
                    continue;

                Form1.packages_.AddToListView(rec);
            }

            Form1.packages_.EndListViewUpdate();
        }

        void PopulateAttachmentView(int filter)
        {
            Form1.attachments_.BeginListViewUpdate();

            foreach (DBAttachment rec in Form1.attachments_.data_.Values)
            {
                Form1.attachments_.AddToListView(rec);
            }

            Form1.attachments_.EndListViewUpdate();
        }

        void ToggleFormsVisibility(ListView lv, EditRecord er)
        {
            listViewG.Hide();
            listViewI.Hide();
            listViewW.Hide();
            listViewP.Hide();
            listViewA.Hide();
            editG_.Hide();
            editW_.Hide();
            editU_.Hide();
            editI_.Hide();
            editP_.Hide();
            editA_.Hide();

            curListView_ = lv;
            if (lv == null)
                return;

            lv.Show();

            // 'er' will be shown in OnListView_SelectedIndexChanged
            if (lv.Items.Count > 0)
            {
                lv.Focus();
                lv.SelectedIndices.Clear();
                lv.Items[0].Selected = true;
            }
        }

        private void cbWeapType_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (Form1.weapons_.data_.Count == 0)
                return;

            if (cbWeapType.SelectedIndex == 0)
            {
                ToggleFormsVisibility(null, null);
                return;
            }

            int gearCategory = cbItemType[cbWeapType.SelectedIndex];
            if (DBWeapon.IS_THIS_CATEGORY(gearCategory))
            {
                PopulateWeaponsView(gearCategory);
                if (gearCategory == 28)
                {
                    // special edit case for usable items
                    ToggleFormsVisibility(listViewW, editU_);
                    btnNewItem.Text = "New Usable Item";
                }
                else
                {
                    ToggleFormsVisibility(listViewW, editW_);
                    btnNewItem.Text = "New Weapon";
                }
            }
            else if (DBGear.IS_THIS_CATEGORY(gearCategory))
            {
                PopulateGearsView(gearCategory);
                ToggleFormsVisibility(listViewG, editG_);
                btnNewItem.Text = "New Gear";
            }
            else if (DBItem.IS_THIS_CATEGORY(gearCategory))
            {
                PopulateItemsView(gearCategory);
                ToggleFormsVisibility(listViewI, editI_);
                btnNewItem.Text = "New Item";
            }
            else if (DBPackage.IS_THIS_CATEGORY(gearCategory))
            {
                PopulatePackagesView(gearCategory);
                ToggleFormsVisibility(listViewP, editP_);
                btnNewItem.Text = "New Package";
            }
            else if (DBAttachment.IS_THIS_CATEGORY(gearCategory))
            {
                PopulateAttachmentView(gearCategory);
                ToggleFormsVisibility(listViewA, editA_);
                btnNewItem.Text = "New Attachment";
            }
            else 
            {
                throw new ArgumentException("category " + gearCategory + " unknown");
            }
        }

        void OnListView_SelectedIndexChanged(ListView lv, EditRecord er)
        {
            if (lv.SelectedItems.Count == 0)
            {
                er.DisableAll();
                editSku_.DisableAll();
                return;
            }

            SQLRec rec = lv.SelectedItems[0].Tag as SQLRec;
            er.Show();
            er.SetFromRecord(rec);

            // if editing mystery crate, show edit loot button
            if ((rec as DBItem) != null)
            {
                if ((rec as DBItem).Is_Mystery_Crate())
                {
                    int cnt = EditLootDrop.GetItemsCount(rec.RECORD_ID.AsInt());
                    lblLootBox_.Text = string.Format("{0} items", cnt);
                    lblLootBox_.Show();
                    btnLootBox_.Show();
                }
                else
                {
                    lblLootBox_.Hide();
                    btnLootBox_.Hide();
                }
            }

            editSku_.Show();
            editSku_.SetFromPrice(rec as DBCommonItem);
        }

        private void listViewG_SelectedIndexChanged(object sender, EventArgs e)
        {
            OnListView_SelectedIndexChanged(listViewG, editG_);
        }

        private void listViewW_SelectedIndexChanged(object sender, EventArgs e)
        {
            int gearCategory = cbItemType[cbWeapType.SelectedIndex];
            if(gearCategory == 28)
                OnListView_SelectedIndexChanged(listViewW, editU_);
            else
                OnListView_SelectedIndexChanged(listViewW, editW_);
        }

        private void listViewI_SelectedIndexChanged(object sender, EventArgs e)
        {
            OnListView_SelectedIndexChanged(listViewI, editI_);
        }

        private void listViewP_SelectedIndexChanged(object sender, EventArgs e)
        {
            OnListView_SelectedIndexChanged(listViewP, editP_);
        }

        private void listViewA_SelectedIndexChanged(object sender, EventArgs e)
        {
            OnListView_SelectedIndexChanged(listViewA, editA_);
        }

        private void btnNewItem_Click(object sender, EventArgs e)
        {
            int gearCategory = cbItemType[cbWeapType.SelectedIndex];
            if(DBWeapon.IS_THIS_CATEGORY(gearCategory))
            {
                FormAddWeapon form = new FormAddWeapon(gearCategory);
                form.ShowDialog();
                form.Dispose();
            }
            else if (DBGear.IS_THIS_CATEGORY(gearCategory))
            {
                FormAddGear form = new FormAddGear(gearCategory);
                form.ShowDialog();
                form.Dispose();
            }
            else if (DBItem.IS_THIS_CATEGORY(gearCategory))
            {
                FormAddItem form = new FormAddItem(gearCategory);
                form.ShowDialog();
                form.Dispose();
            }
            else if (DBAttachment.IS_THIS_CATEGORY(gearCategory))
            {
                FormAddAttachment form = new FormAddAttachment(gearCategory);
                form.ShowDialog();
                form.Dispose();
            }
            else if (DBPackage.IS_THIS_CATEGORY(gearCategory))
            {
                FormAddPackage form = new FormAddPackage(gearCategory);
                form.ShowDialog();
                form.Dispose();
            }
            else
            {
                throw new ArgumentException("category " + gearCategory + " unknown");
            }
        }

        private void btnFindItemID_Click(object sender, EventArgs e)
        {
            try
            {
                int ItemID = Convert.ToInt32(tbItemID.Text);

                string name = "";
                string desc = "";
                int cat = 0;
                if (!Form1.THIS.GetItemInfo(ItemID, ref name, ref desc, ref cat))
                {
                    throw new ArgumentException("no such ItemID");
                }

                for (int i = 0; i < cbItemType.Length; i++)
                {
                    if (cbItemType[i] == cat)
                    {
                        cbWeapType.SelectedIndex = i;
                        foreach (ListViewItem lvi in curListView_.Items)
                        {
                            SQLRec rec = lvi.Tag as SQLRec;
                            if (rec.RECORD_ID.AsInt() == ItemID)
                            {
                                lvi.EnsureVisible();
                                curListView_.SelectedIndices.Clear();
                                lvi.Selected = true;
                                return;
                            }
                        }
                        break;
                    }
                }
                throw new ArgumentException("crap happens");
            }
            catch (Exception _e)
            {
                MessageBox.Show(_e.Message);
            }
        }

        private void tbItemID_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Enter)
                btnFindItemID_Click(this, new EventArgs());
        }
    }
}
