namespace WOAdmin
{
    partial class ViewUsers
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
            this.components = new System.ComponentModel.Container();
            this.listView1 = new System.Windows.Forms.ListView();
            this.columnHeader1 = new System.Windows.Forms.ColumnHeader();
            this.label1 = new System.Windows.Forms.Label();
            this.tbFilter1 = new System.Windows.Forms.TextBox();
            this.button1 = new System.Windows.Forms.Button();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.btnFilter1 = new System.Windows.Forms.Button();
            this.btnFilter2 = new System.Windows.Forms.Button();
            this.tbFilter2 = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.btnFilter3 = new System.Windows.Forms.Button();
            this.tbFilter3 = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.btnFilter4 = new System.Windows.Forms.Button();
            this.tbFilter4 = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.contextMenuStrip1 = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripMenuItem();
            this.tEMPResetAllFPSWeaponsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.tbFilter5 = new System.Windows.Forms.TextBox();
            this.btnFilter5 = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.btnAddItem = new System.Windows.Forms.Button();
            this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.contextMenuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // listView1
            // 
            this.listView1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.listView1.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.listView1.FullRowSelect = true;
            this.listView1.GridLines = true;
            this.listView1.Location = new System.Drawing.Point(24, 23);
            this.listView1.Name = "listView1";
            this.listView1.Size = new System.Drawing.Size(656, 109);
            this.listView1.TabIndex = 0;
            this.listView1.UseCompatibleStateImageBehavior = false;
            this.listView1.View = System.Windows.Forms.View.Details;
            this.listView1.SelectedIndexChanged += new System.EventHandler(this.listView1_SelectedIndexChanged);
            this.listView1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.listView1_MouseDown);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "users...";
            this.columnHeader1.Width = 141;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(0, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(75, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "AccountName";
            // 
            // tbFilter1
            // 
            this.tbFilter1.Location = new System.Drawing.Point(81, 5);
            this.tbFilter1.Name = "tbFilter1";
            this.tbFilter1.Size = new System.Drawing.Size(222, 20);
            this.tbFilter1.TabIndex = 3;
            this.tbFilter1.KeyDown += new System.Windows.Forms.KeyEventHandler(this.tbFilter1_KeyDown);
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.button1.Location = new System.Drawing.Point(633, 3);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(95, 23);
            this.button1.TabIndex = 4;
            this.button1.Text = "Add New User";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.splitContainer1.Location = new System.Drawing.Point(3, 120);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.listView1);
            this.splitContainer1.Size = new System.Drawing.Size(745, 397);
            this.splitContainer1.SplitterDistance = 196;
            this.splitContainer1.TabIndex = 6;
            // 
            // btnFilter1
            // 
            this.btnFilter1.Location = new System.Drawing.Point(309, 3);
            this.btnFilter1.Name = "btnFilter1";
            this.btnFilter1.Size = new System.Drawing.Size(64, 23);
            this.btnFilter1.TabIndex = 7;
            this.btnFilter1.Text = "Search";
            this.btnFilter1.UseVisualStyleBackColor = true;
            this.btnFilter1.Click += new System.EventHandler(this.btnFilter1_Click);
            // 
            // btnFilter2
            // 
            this.btnFilter2.Location = new System.Drawing.Point(309, 46);
            this.btnFilter2.Name = "btnFilter2";
            this.btnFilter2.Size = new System.Drawing.Size(64, 23);
            this.btnFilter2.TabIndex = 16;
            this.btnFilter2.Text = "Search";
            this.btnFilter2.UseVisualStyleBackColor = true;
            this.btnFilter2.Click += new System.EventHandler(this.btnFilter2_Click);
            // 
            // tbFilter2
            // 
            this.tbFilter2.Location = new System.Drawing.Point(81, 48);
            this.tbFilter2.Name = "tbFilter2";
            this.tbFilter2.Size = new System.Drawing.Size(222, 20);
            this.tbFilter2.TabIndex = 15;
            this.tbFilter2.KeyDown += new System.Windows.Forms.KeyEventHandler(this.tbFilter2_KeyDown);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(22, 50);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(53, 13);
            this.label4.TabIndex = 14;
            this.label4.Text = "Gamertag";
            // 
            // btnFilter3
            // 
            this.btnFilter3.Location = new System.Drawing.Point(309, 69);
            this.btnFilter3.Name = "btnFilter3";
            this.btnFilter3.Size = new System.Drawing.Size(64, 23);
            this.btnFilter3.TabIndex = 19;
            this.btnFilter3.Text = "Search";
            this.btnFilter3.UseVisualStyleBackColor = true;
            this.btnFilter3.Click += new System.EventHandler(this.btnFilter3_Click);
            // 
            // tbFilter3
            // 
            this.tbFilter3.Location = new System.Drawing.Point(81, 71);
            this.tbFilter3.Name = "tbFilter3";
            this.tbFilter3.Size = new System.Drawing.Size(222, 20);
            this.tbFilter3.TabIndex = 18;
            this.tbFilter3.KeyDown += new System.Windows.Forms.KeyEventHandler(this.tbFilter3_KeyDown);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(41, 73);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(34, 13);
            this.label5.TabIndex = 17;
            this.label5.Text = "e-mail";
            // 
            // btnFilter4
            // 
            this.btnFilter4.Location = new System.Drawing.Point(309, 92);
            this.btnFilter4.Name = "btnFilter4";
            this.btnFilter4.Size = new System.Drawing.Size(64, 23);
            this.btnFilter4.TabIndex = 22;
            this.btnFilter4.Text = "Search";
            this.btnFilter4.UseVisualStyleBackColor = true;
            this.btnFilter4.Click += new System.EventHandler(this.btnFilter4_Click);
            // 
            // tbFilter4
            // 
            this.tbFilter4.Location = new System.Drawing.Point(81, 94);
            this.tbFilter4.Name = "tbFilter4";
            this.tbFilter4.Size = new System.Drawing.Size(222, 20);
            this.tbFilter4.TabIndex = 21;
            this.tbFilter4.KeyDown += new System.Windows.Forms.KeyEventHandler(this.tbFilter4_KeyDown);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(58, 96);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(17, 13);
            this.label6.TabIndex = 20;
            this.label6.Text = "IP";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(379, 7);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(179, 13);
            this.label2.TabIndex = 24;
            this.label2.Text = "Use =XXX to search for exact match";
            // 
            // contextMenuStrip1
            // 
            this.contextMenuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripSeparator1,
            this.toolStripMenuItem1,
            this.toolStripMenuItem2,
            this.toolStripSeparator2,
            this.tEMPResetAllFPSWeaponsToolStripMenuItem});
            this.contextMenuStrip1.Name = "contextMenuStrip1";
            this.contextMenuStrip1.Size = new System.Drawing.Size(227, 104);
            // 
            // toolStripSeparator1
            // 
            this.toolStripSeparator1.Name = "toolStripSeparator1";
            this.toolStripSeparator1.Size = new System.Drawing.Size(223, 6);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(226, 22);
            this.toolStripMenuItem1.Text = "View Items";
            this.toolStripMenuItem1.Click += new System.EventHandler(this.toolStripMenuItem1_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(226, 22);
            this.toolStripMenuItem2.Text = "Add Item";
            this.toolStripMenuItem2.Click += new System.EventHandler(this.toolStripMenuItem2_Click);
            // 
            // tEMPResetAllFPSWeaponsToolStripMenuItem
            // 
            this.tEMPResetAllFPSWeaponsToolStripMenuItem.Name = "tEMPResetAllFPSWeaponsToolStripMenuItem";
            this.tEMPResetAllFPSWeaponsToolStripMenuItem.Size = new System.Drawing.Size(226, 22);
            this.tEMPResetAllFPSWeaponsToolStripMenuItem.Text = "TEMP: Reset all FPS weapons";
            this.tEMPResetAllFPSWeaponsToolStripMenuItem.Click += new System.EventHandler(this.tEMPResetAllFPSWeaponsToolStripMenuItem_Click);
            // 
            // tbFilter5
            // 
            this.tbFilter5.Location = new System.Drawing.Point(81, 26);
            this.tbFilter5.Name = "tbFilter5";
            this.tbFilter5.Size = new System.Drawing.Size(222, 20);
            this.tbFilter5.TabIndex = 26;
            this.tbFilter5.KeyDown += new System.Windows.Forms.KeyEventHandler(this.tbFilter5_KeyDown);
            // 
            // btnFilter5
            // 
            this.btnFilter5.Location = new System.Drawing.Point(309, 24);
            this.btnFilter5.Name = "btnFilter5";
            this.btnFilter5.Size = new System.Drawing.Size(64, 23);
            this.btnFilter5.TabIndex = 27;
            this.btnFilter5.Text = "Search";
            this.btnFilter5.UseVisualStyleBackColor = true;
            this.btnFilter5.Click += new System.EventHandler(this.btnFilter5_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(12, 29);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(62, 13);
            this.label3.TabIndex = 25;
            this.label3.Text = "CustomerID";
            // 
            // btnAddItem
            // 
            this.btnAddItem.Location = new System.Drawing.Point(590, 44);
            this.btnAddItem.Name = "btnAddItem";
            this.btnAddItem.Size = new System.Drawing.Size(75, 23);
            this.btnAddItem.TabIndex = 28;
            this.btnAddItem.Text = "Add Items";
            this.btnAddItem.UseVisualStyleBackColor = true;
            this.btnAddItem.Click += new System.EventHandler(this.btnAddItem_Click);
            // 
            // toolStripSeparator2
            // 
            this.toolStripSeparator2.Name = "toolStripSeparator2";
            this.toolStripSeparator2.Size = new System.Drawing.Size(223, 6);
            // 
            // ViewUsers
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(750, 521);
            this.Controls.Add(this.btnAddItem);
            this.Controls.Add(this.tbFilter5);
            this.Controls.Add(this.btnFilter5);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.tbFilter4);
            this.Controls.Add(this.btnFilter4);
            this.Controls.Add(this.label6);
            this.Controls.Add(this.tbFilter3);
            this.Controls.Add(this.btnFilter3);
            this.Controls.Add(this.label5);
            this.Controls.Add(this.tbFilter2);
            this.Controls.Add(this.btnFilter2);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.splitContainer1);
            this.Controls.Add(this.btnFilter1);
            this.Controls.Add(this.button1);
            this.Controls.Add(this.tbFilter1);
            this.Controls.Add(this.label1);
            this.Name = "ViewUsers";
            this.Text = "ViewUsers";
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.ResumeLayout(false);
            this.contextMenuStrip1.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ListView listView1;
        private System.Windows.Forms.ColumnHeader columnHeader1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.TextBox tbFilter1;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.Button btnFilter1;
        private System.Windows.Forms.Button btnFilter2;
        private System.Windows.Forms.TextBox tbFilter2;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button btnFilter3;
        private System.Windows.Forms.TextBox tbFilter3;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button btnFilter4;
        private System.Windows.Forms.TextBox tbFilter4;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.ContextMenuStrip contextMenuStrip1;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem1;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
        private System.Windows.Forms.TextBox tbFilter5;
        private System.Windows.Forms.Button btnFilter5;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.ToolStripMenuItem toolStripMenuItem2;
        private System.Windows.Forms.Button btnAddItem;
        private System.Windows.Forms.ToolStripMenuItem tEMPResetAllFPSWeaponsToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
    }
}