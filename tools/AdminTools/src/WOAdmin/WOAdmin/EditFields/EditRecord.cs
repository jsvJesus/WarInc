using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace WOAdmin
{
    public partial class EditRecord : Form
    {
        protected class EditField
        {
            public bool readOnly = false;
            public string name = "";
            public SQLField fld = null;

            public Control label = null;
            public Control edit = null;
            public int temp_oldheight = 20;

            public EditField RO(bool set)
            {
                readOnly = set;
                edit.Enabled = !set;
                return this;
            }
        };
        protected List<EditField> fields_ = new List<EditField>();
        public SQLRec sqlrec_ = null;

        private Button btnDelete_ = null;
        private Button btnSave_ = null;

        public EditRecord()
        {
        }

        Control GetLabel(string name)
        {
            foreach (Control cnt in this.Controls)
            {
                if (cnt as Label == null)
                    continue;
                if (cnt.Text == name)
                    return cnt;
            }

            return null;
        }

        Control GetEditControlByType(SQLDataType type)
        {
            switch (type)
            {
                case SQLDataType.STRING:
                    return new EditField_Text();
                case SQLDataType.INT:
                    return new EditField_Int();
                case SQLDataType.DOUBLE:
                    return new EditField_Float();
                case SQLDataType.DATETIME:
                    return new EditField_DateTime();
            }
            throw new ArgumentException("bad type in GetEditControlByType");
        }

        protected EditField Reg(string name, SQLDataType type)
        {
            Control c = GetLabel(name);
            if (c == null)
                return null;

            return Reg(name, type, (Control)null);
        }

        protected EditField Reg(string name, SQLDataType type, string label)
        {
            Control c = GetLabel(name);
            if (c == null)
                return null;

            EditField ef = Reg(name, type, (Control)null);
            ef.label.Text = label;
            return ef;
        }

        protected EditField Reg(string name, SQLDataType type, Control e)
        {
            Label l = GetLabel(name) as Label;
            if (l == null)
                return null;

            const int EDIT_OFFSET = 90;

            l.BackColor = System.Drawing.SystemColors.Control;
            int editWidth = l.Size.Width - EDIT_OFFSET;

            if (e == null)
            {
                e = GetEditControlByType(type);
            }

            e.Text = "";
            e.Location = new Point(l.Location.X + EDIT_OFFSET, l.Location.Y - 2);
            e.Size = new Size(editWidth, l.Size.Height);
            e.BackColor = System.Drawing.SystemColors.Control;
            this.Controls.Add(e);

            EditField ef = new EditField();
            ef.name = name;
            ef.edit = e;
            ef.label = l;
            ef.temp_oldheight = l.Size.Height;
            fields_.Add(ef);

            l.AutoSize = true;

            // fix REC:NAME labels
            string[] parts = name.Split(":".ToCharArray());
            if (parts.Length > 1)
                l.Text = parts[1];

            return ef;
        }

        public void AttachSaveButton(System.EventHandler handler)
        {
            System.Diagnostics.Debug.Assert(btnSave_ == null);
            btnSave_ = new System.Windows.Forms.Button();
            btnSave_.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            btnSave_.BackColor = System.Drawing.SystemColors.ControlLight;
            btnSave_.Location = new System.Drawing.Point(this.Width - 100, 9);
            btnSave_.Name = "btnSave";
            btnSave_.Size = new System.Drawing.Size(75, 23);
            btnSave_.TabIndex = 15;
            btnSave_.Text = "Save";
            btnSave_.UseVisualStyleBackColor = false;
            btnSave_.Click += handler;
            this.Controls.Add(btnSave_);
        }

        public void AttachDeleteButton(System.EventHandler handler)
        {
            System.Diagnostics.Debug.Assert(btnDelete_ == null);
            btnDelete_ = new System.Windows.Forms.Button();
            btnDelete_.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            btnDelete_.BackColor = System.Drawing.SystemColors.ControlLight;
            btnDelete_.Location = new System.Drawing.Point(this.Width - 100, 40);
            btnDelete_.Name = "btnDelete";
            btnDelete_.Size = new System.Drawing.Size(75, 23);
            btnDelete_.TabIndex = 16;
            btnDelete_.Text = "Delete";
            btnDelete_.UseVisualStyleBackColor = false;
            btnDelete_.Click += handler;
            this.Controls.Add(btnDelete_);
        }

        private void EnableAttachedButtons(bool enabled)
        {
            if (btnDelete_ != null)
                btnDelete_.Enabled = enabled;
            if (btnSave_ != null)
                btnSave_.Enabled = enabled;
        }

        virtual public void SetFromRecord(SQLRec rec)
        {
            sqlrec_ = rec;
            EnableAttachedButtons(true);

            if (rec.RECORD_ID.type2 != SQLDataType.NONE)
            {
                // first entry MUST be ID
                (fields_[0].edit as IEditField).FldSet(rec.RECORD_ID);
                fields_[0].readOnly = true;
            }

            foreach (EditField ef in fields_)
            {
                ef.edit.Enabled = true;
                if (ef.readOnly)
                {
                    if ((ef.edit as TextBox) == null)
                    {
                        ef.edit.Enabled = false;
                    }
                    else
                    {
                        (ef.edit as TextBox).ReadOnly = ef.readOnly;
                        (ef.edit as TextBox).ForeColor = Color.DarkGray;
                    }
                }

                SQLField fld = rec.GetField(ef.name);
                if (fld == null)
                    continue;

                ef.fld = fld;
                (ef.edit as IEditField).FldSet(fld);
            }
        }

        public void DisableAll()
        {
            sqlrec_ = null;
            EnableAttachedButtons(false);

            foreach (EditField ef in fields_)
            {
                ef.edit.Enabled = false;
                ef.fld = null;
                (ef.edit as IEditField).FldReset();
            }
        }

        public bool ValidateEdit()
        {
            string allmsgs = "";
            string msg = "";

            foreach (EditField ef in fields_)
            {
                if (ef.readOnly)
                    continue;
                if ((ef.edit as IEditField).FldIsValid(ref msg) == false)
                {
                    allmsgs += string.Format("{0} invalid: {1}\n",
                        ef.name, msg);
                }
            }

            if (allmsgs.Length > 0)
            {
                MessageBox.Show(allmsgs, "Bad fields");
                return false;
            }

            return true;
        }

        protected void FinishEdit()
        {
            foreach (EditField ef in fields_)
            {
                if (ef.readOnly)
                    continue;
                if (ef.fld == null)
                    continue;

                (ef.edit as IEditField).FldGet(ef.fld);
            }

            return;
        }

        public bool ERDelete()
        {
            if (sqlrec_ == null)
                return false;

            if(MessageBox.Show("Are you sure you want to delete this record?", 
                "DELETING RECORD", MessageBoxButtons.YesNo) != DialogResult.Yes)
            {
                return false;
            }

            if (!Form1.sql.Delete(sqlrec_))
                return false;

            // remove from listview
            if (sqlrec_.ui_lvi != null)
            {
                sqlrec_.ui_lvi.Tag = null;
                sqlrec_.ui_lvi.Remove();
            }

            DisableAll();
            return true;
        }

        virtual public bool ERSave()
        {
            if (sqlrec_ == null)
                return false;

            if(!ValidateEdit())
                return false;

            FinishEdit();

            Form1.sql.Update(sqlrec_);
            return true;
        }
    }
}
