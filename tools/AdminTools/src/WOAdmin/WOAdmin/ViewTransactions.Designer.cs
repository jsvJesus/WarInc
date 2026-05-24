namespace WOAdmin
{
    partial class ViewTransactions
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.cbTransType = new System.Windows.Forms.ComboBox();
            this.dtpickFrom = new System.Windows.Forms.DateTimePicker();
            this.tbCustomerID = new System.Windows.Forms.TextBox();
            this.tbItemID = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label6 = new System.Windows.Forms.Label();
            this.label7 = new System.Windows.Forms.Label();
            this.dtpickTo = new System.Windows.Forms.DateTimePicker();
            this.listViewR = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.btnUpdate = new System.Windows.Forms.Button();
            this.listViewG = new System.Windows.Forms.ListView();
            this.columnHeader7 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader9 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader10 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader11 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader14 = new System.Windows.Forms.ColumnHeader();
            this.columnHeader12 = new System.Windows.Forms.ColumnHeader();
            this.textBoxCustomerID = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // cbTransType
            // 
            this.cbTransType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbTransType.FormattingEnabled = true;
            this.cbTransType.Location = new System.Drawing.Point(93, 12);
            this.cbTransType.Name = "cbTransType";
            this.cbTransType.Size = new System.Drawing.Size(143, 21);
            this.cbTransType.TabIndex = 2;
            this.cbTransType.SelectedIndexChanged += new System.EventHandler(this.cbTransType_SelectedIndexChanged);
            // 
            // dtpickFrom
            // 
            this.dtpickFrom.CustomFormat = "";
            this.dtpickFrom.Format = System.Windows.Forms.DateTimePickerFormat.Short;
            this.dtpickFrom.Location = new System.Drawing.Point(93, 65);
            this.dtpickFrom.Name = "dtpickFrom";
            this.dtpickFrom.Size = new System.Drawing.Size(143, 20);
            this.dtpickFrom.TabIndex = 4;
            this.dtpickFrom.Value = new System.DateTime(2010, 1, 1, 0, 0, 0, 0);
            // 
            // tbCustomerID
            // 
            this.tbCustomerID.Location = new System.Drawing.Point(93, 39);
            this.tbCustomerID.Name = "tbCustomerID";
            this.tbCustomerID.Size = new System.Drawing.Size(143, 20);
            this.tbCustomerID.TabIndex = 6;
            // 
            // tbItemID
            // 
            this.tbItemID.Location = new System.Drawing.Point(93, 117);
            this.tbItemID.Name = "tbItemID";
            this.tbItemID.Size = new System.Drawing.Size(143, 20);
            this.tbItemID.TabIndex = 8;
            // 
            // label4
            // 
            this.label4.Location = new System.Drawing.Point(12, 68);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(78, 13);
            this.label4.TabIndex = 9;
            this.label4.Text = "From:";
            this.label4.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label3
            // 
            this.label3.Location = new System.Drawing.Point(9, 117);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(78, 13);
            this.label3.TabIndex = 12;
            this.label3.Text = "ItemID:";
            this.label3.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label5
            // 
            this.label5.Location = new System.Drawing.Point(12, 94);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(78, 13);
            this.label5.TabIndex = 13;
            this.label5.Text = "To:";
            this.label5.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label6
            // 
            this.label6.Location = new System.Drawing.Point(9, 42);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(78, 13);
            this.label6.TabIndex = 14;
            this.label6.Text = "Customer ID:";
            this.label6.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // label7
            // 
            this.label7.Location = new System.Drawing.Point(9, 15);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(78, 13);
            this.label7.TabIndex = 15;
            this.label7.Text = "Type:";
            this.label7.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // dtpickTo
            // 
            this.dtpickTo.CustomFormat = "";
            this.dtpickTo.Format = System.Windows.Forms.DateTimePickerFormat.Short;
            this.dtpickTo.Location = new System.Drawing.Point(93, 91);
            this.dtpickTo.Name = "dtpickTo";
            this.dtpickTo.Size = new System.Drawing.Size(143, 20);
            this.dtpickTo.TabIndex = 16;
            this.dtpickTo.Value = new System.DateTime(2014, 1, 1, 0, 0, 0, 0);
            // 
            // listViewR
            // 
            this.listViewR.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewR.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.listViewR.FullRowSelect = true;
            this.listViewR.GridLines = true;
            this.listViewR.Location = new System.Drawing.Point(2, 143);
            this.listViewR.MultiSelect = false;
            this.listViewR.Name = "listViewR";
            this.listViewR.Size = new System.Drawing.Size(740, 70);
            this.listViewR.TabIndex = 17;
            this.listViewR.UseCompatibleStateImageBehavior = false;
            this.listViewR.View = System.Windows.Forms.View.Details;
            this.listViewR.SelectedIndexChanged += new System.EventHandler(this.listViewR_SelectedIndexChanged);
            this.listViewR.KeyDown += new System.Windows.Forms.KeyEventHandler(this.listViewR_KeyDown);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "real money listview";
            this.columnHeader1.Width = 252;
            // 
            // btnUpdate
            // 
            this.btnUpdate.Location = new System.Drawing.Point(281, 42);
            this.btnUpdate.Name = "btnUpdate";
            this.btnUpdate.Size = new System.Drawing.Size(69, 53);
            this.btnUpdate.TabIndex = 18;
            this.btnUpdate.Text = "Update";
            this.btnUpdate.UseVisualStyleBackColor = true;
            this.btnUpdate.Click += new System.EventHandler(this.btnUpdate_Click);
            // 
            // listViewG
            // 
            this.listViewG.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewG.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader7,
            this.columnHeader9,
            this.columnHeader10,
            this.columnHeader11,
            this.columnHeader14,
            this.columnHeader12});
            this.listViewG.FullRowSelect = true;
            this.listViewG.GridLines = true;
            this.listViewG.Location = new System.Drawing.Point(2, 219);
            this.listViewG.Name = "listViewG";
            this.listViewG.Size = new System.Drawing.Size(740, 70);
            this.listViewG.TabIndex = 19;
            this.listViewG.UseCompatibleStateImageBehavior = false;
            this.listViewG.View = System.Windows.Forms.View.Details;
            // 
            // columnHeader7
            // 
            this.columnHeader7.Text = "CustomerID";
            this.columnHeader7.Width = 108;
            // 
            // columnHeader9
            // 
            this.columnHeader9.Text = "Type";
            this.columnHeader9.Width = 111;
            // 
            // columnHeader10
            // 
            this.columnHeader10.Text = "DateTime";
            this.columnHeader10.Width = 122;
            // 
            // columnHeader11
            // 
            this.columnHeader11.Text = "Amount";
            // 
            // columnHeader14
            // 
            this.columnHeader14.Text = "ItemID";
            this.columnHeader14.Width = 124;
            // 
            // columnHeader12
            // 
            this.columnHeader12.Text = "ItemDesc";
            this.columnHeader12.Width = 199;
            // 
            // textBoxCustomerID
            // 
            this.textBoxCustomerID.Location = new System.Drawing.Point(531, 15);
            this.textBoxCustomerID.Name = "textBoxCustomerID";
            this.textBoxCustomerID.ReadOnly = true;
            this.textBoxCustomerID.Size = new System.Drawing.Size(143, 20);
            this.textBoxCustomerID.TabIndex = 20;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(460, 18);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(65, 13);
            this.label1.TabIndex = 21;
            this.label1.Text = "CustomerID:";
            // 
            // ViewTransactions
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(745, 461);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.textBoxCustomerID);
            this.Controls.Add(this.listViewG);
            this.Controls.Add(this.btnUpdate);
            this.Controls.Add(this.listViewR);
            this.Controls.Add(this.dtpickTo);
            this.Controls.Add(this.label7);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.tbItemID);
            this.Controls.Add(this.tbCustomerID);
            this.Controls.Add(this.dtpickFrom);
            this.Controls.Add(this.cbTransType);
            this.Name = "ViewTransactions";
            this.Text = "ViewTransactions";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbTransType;
        private System.Windows.Forms.DateTimePicker dtpickFrom;
        private System.Windows.Forms.TextBox tbCustomerID;
        private System.Windows.Forms.TextBox tbItemID;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.DateTimePicker dtpickTo;
        private System.Windows.Forms.ListView listViewR;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Button btnUpdate;
        private System.Windows.Forms.ListView listViewG;
        private System.Windows.Forms.ColumnHeader columnHeader7;
        private System.Windows.Forms.ColumnHeader columnHeader10;
        private System.Windows.Forms.ColumnHeader columnHeader11;
        private System.Windows.Forms.ColumnHeader columnHeader14;
        private System.Windows.Forms.ColumnHeader columnHeader9;
        private System.Windows.Forms.ColumnHeader columnHeader12;
        private System.Windows.Forms.TextBox textBoxCustomerID;
        private System.Windows.Forms.Label label1;

    }
}