namespace WOAdmin
{
    partial class ViewItems
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
            this.cbWeapType = new System.Windows.Forms.ComboBox();
            this.label1 = new System.Windows.Forms.Label();
            this.listViewG = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.listViewW = new System.Windows.Forms.ListView();
            this.columnHeader3 = new System.Windows.Forms.ColumnHeader();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.listViewA = new System.Windows.Forms.ListView();
            this.columnHeader5 = new System.Windows.Forms.ColumnHeader();
            this.listViewP = new System.Windows.Forms.ListView();
            this.columnHeader4 = new System.Windows.Forms.ColumnHeader();
            this.listViewI = new System.Windows.Forms.ListView();
            this.columnHeader2 = new System.Windows.Forms.ColumnHeader();
            this.splitContainer2 = new System.Windows.Forms.SplitContainer();
            this.btnNewItem = new System.Windows.Forms.Button();
            this.label2 = new System.Windows.Forms.Label();
            this.tbItemID = new System.Windows.Forms.TextBox();
            this.btnFindItemID = new System.Windows.Forms.Button();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.splitContainer2.SuspendLayout();
            this.SuspendLayout();
            // 
            // cbWeapType
            // 
            this.cbWeapType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.cbWeapType.FormattingEnabled = true;
            this.cbWeapType.Location = new System.Drawing.Point(52, 6);
            this.cbWeapType.Name = "cbWeapType";
            this.cbWeapType.Size = new System.Drawing.Size(225, 21);
            this.cbWeapType.TabIndex = 0;
            this.cbWeapType.SelectedIndexChanged += new System.EventHandler(this.cbWeapType_SelectedIndexChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 9);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Type:";
            // 
            // listViewG
            // 
            this.listViewG.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewG.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.listViewG.FullRowSelect = true;
            this.listViewG.GridLines = true;
            this.listViewG.Location = new System.Drawing.Point(14, 3);
            this.listViewG.Name = "listViewG";
            this.listViewG.Size = new System.Drawing.Size(595, 52);
            this.listViewG.TabIndex = 2;
            this.listViewG.UseCompatibleStateImageBehavior = false;
            this.listViewG.View = System.Windows.Forms.View.Details;
            this.listViewG.SelectedIndexChanged += new System.EventHandler(this.listViewG_SelectedIndexChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "gear...";
            this.columnHeader1.Width = 179;
            // 
            // listViewW
            // 
            this.listViewW.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewW.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader3});
            this.listViewW.FullRowSelect = true;
            this.listViewW.GridLines = true;
            this.listViewW.Location = new System.Drawing.Point(14, 61);
            this.listViewW.Name = "listViewW";
            this.listViewW.Size = new System.Drawing.Size(595, 50);
            this.listViewW.TabIndex = 3;
            this.listViewW.UseCompatibleStateImageBehavior = false;
            this.listViewW.View = System.Windows.Forms.View.Details;
            this.listViewW.SelectedIndexChanged += new System.EventHandler(this.listViewW_SelectedIndexChanged);
            // 
            // columnHeader3
            // 
            this.columnHeader3.Text = "wepon...";
            this.columnHeader3.Width = 180;
            // 
            // splitContainer1
            // 
            this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainer1.Location = new System.Drawing.Point(1, 33);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listViewA);
            this.splitContainer1.Panel1.Controls.Add(this.listViewP);
            this.splitContainer1.Panel1.Controls.Add(this.listViewI);
            this.splitContainer1.Panel1.Controls.Add(this.listViewW);
            this.splitContainer1.Panel1.Controls.Add(this.listViewG);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.splitContainer2);
            this.splitContainer1.Size = new System.Drawing.Size(1097, 434);
            this.splitContainer1.SplitterDistance = 123;
            this.splitContainer1.TabIndex = 4;
            // 
            // listViewA
            // 
            this.listViewA.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewA.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader5});
            this.listViewA.FullRowSelect = true;
            this.listViewA.GridLines = true;
            this.listViewA.Location = new System.Drawing.Point(238, 3);
            this.listViewA.Name = "listViewA";
            this.listViewA.Size = new System.Drawing.Size(595, 52);
            this.listViewA.TabIndex = 6;
            this.listViewA.UseCompatibleStateImageBehavior = false;
            this.listViewA.View = System.Windows.Forms.View.Details;
            this.listViewA.SelectedIndexChanged += new System.EventHandler(this.listViewA_SelectedIndexChanged);
            // 
            // columnHeader5
            // 
            this.columnHeader5.Text = "attachments...";
            this.columnHeader5.Width = 179;
            // 
            // listViewP
            // 
            this.listViewP.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewP.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader4});
            this.listViewP.FullRowSelect = true;
            this.listViewP.GridLines = true;
            this.listViewP.Location = new System.Drawing.Point(441, 61);
            this.listViewP.Name = "listViewP";
            this.listViewP.Size = new System.Drawing.Size(574, 50);
            this.listViewP.TabIndex = 5;
            this.listViewP.UseCompatibleStateImageBehavior = false;
            this.listViewP.View = System.Windows.Forms.View.Details;
            this.listViewP.SelectedIndexChanged += new System.EventHandler(this.listViewP_SelectedIndexChanged);
            // 
            // columnHeader4
            // 
            this.columnHeader4.Text = "packages...";
            this.columnHeader4.Width = 180;
            // 
            // listViewI
            // 
            this.listViewI.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listViewI.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader2});
            this.listViewI.FullRowSelect = true;
            this.listViewI.GridLines = true;
            this.listViewI.Location = new System.Drawing.Point(238, 61);
            this.listViewI.Name = "listViewI";
            this.listViewI.Size = new System.Drawing.Size(574, 50);
            this.listViewI.TabIndex = 4;
            this.listViewI.UseCompatibleStateImageBehavior = false;
            this.listViewI.View = System.Windows.Forms.View.Details;
            this.listViewI.SelectedIndexChanged += new System.EventHandler(this.listViewI_SelectedIndexChanged);
            // 
            // columnHeader2
            // 
            this.columnHeader2.Text = "items...";
            this.columnHeader2.Width = 180;
            // 
            // splitContainer2
            // 
            this.splitContainer2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer2.Location = new System.Drawing.Point(0, 0);
            this.splitContainer2.Name = "splitContainer2";
            this.splitContainer2.Size = new System.Drawing.Size(1097, 307);
            this.splitContainer2.SplitterDistance = 766;
            this.splitContainer2.TabIndex = 0;
            // 
            // btnNewItem
            // 
            this.btnNewItem.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.btnNewItem.Location = new System.Drawing.Point(996, 6);
            this.btnNewItem.Name = "btnNewItem";
            this.btnNewItem.Size = new System.Drawing.Size(102, 23);
            this.btnNewItem.TabIndex = 5;
            this.btnNewItem.Text = "New";
            this.btnNewItem.UseVisualStyleBackColor = true;
            this.btnNewItem.Click += new System.EventHandler(this.btnNewItem_Click);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(283, 11);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(41, 13);
            this.label2.TabIndex = 7;
            this.label2.Text = "ItemID:";
            // 
            // tbItemID
            // 
            this.tbItemID.Location = new System.Drawing.Point(330, 6);
            this.tbItemID.Name = "tbItemID";
            this.tbItemID.Size = new System.Drawing.Size(100, 20);
            this.tbItemID.TabIndex = 8;
            this.tbItemID.KeyDown += new System.Windows.Forms.KeyEventHandler(this.tbItemID_KeyDown);
            // 
            // btnFindItemID
            // 
            this.btnFindItemID.Location = new System.Drawing.Point(436, 4);
            this.btnFindItemID.Name = "btnFindItemID";
            this.btnFindItemID.Size = new System.Drawing.Size(75, 23);
            this.btnFindItemID.TabIndex = 9;
            this.btnFindItemID.Text = "Find";
            this.btnFindItemID.UseVisualStyleBackColor = true;
            this.btnFindItemID.Click += new System.EventHandler(this.btnFindItemID_Click);
            // 
            // ViewItems
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1100, 468);
            this.Controls.Add(this.btnFindItemID);
            this.Controls.Add(this.tbItemID);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.btnNewItem);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.cbWeapType);
            this.Name = "ViewItems";
            this.Text = "ViewItems";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.splitContainer2.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbWeapType;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.ListView listViewG;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.ListView listViewW;
        private System.Windows.Forms.ColumnHeader columnHeader3;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.SplitContainer splitContainer2;
        private System.Windows.Forms.Button btnNewItem;
        private System.Windows.Forms.ListView listViewI;
        private System.Windows.Forms.ColumnHeader columnHeader2;
        private System.Windows.Forms.ListView listViewP;
        private System.Windows.Forms.ColumnHeader columnHeader4;
        private System.Windows.Forms.ListView listViewA;
        private System.Windows.Forms.ColumnHeader columnHeader5;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tbItemID;
        private System.Windows.Forms.Button btnFindItemID;
    }
}