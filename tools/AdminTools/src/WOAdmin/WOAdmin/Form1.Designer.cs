namespace WOAdmin
{
    partial class Form1
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
            this.panelTasks = new System.Windows.Forms.Panel();
            this.panelView = new System.Windows.Forms.Panel();
            this.button4 = new WOAdmin.GSButton();
            this.button7 = new WOAdmin.GSButton();
            this.button6 = new WOAdmin.GSButton();
            this.button3 = new WOAdmin.GSButton();
            this.button2 = new WOAdmin.GSButton();
            this.button1 = new WOAdmin.GSButton();
            this.panelTasks.SuspendLayout();
            this.SuspendLayout();
            // 
            // panelTasks
            // 
            this.panelTasks.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panelTasks.BackColor = System.Drawing.SystemColors.Control;
            this.panelTasks.BackgroundImage = global::WOAdmin.Properties.Resources.AdminDashBoardBackground;
            this.panelTasks.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.panelTasks.Controls.Add(this.button4);
            this.panelTasks.Controls.Add(this.button7);
            this.panelTasks.Controls.Add(this.button6);
            this.panelTasks.Controls.Add(this.button3);
            this.panelTasks.Controls.Add(this.button2);
            this.panelTasks.Controls.Add(this.button1);
            this.panelTasks.Location = new System.Drawing.Point(0, -38);
            this.panelTasks.Name = "panelTasks";
            this.panelTasks.Size = new System.Drawing.Size(1216, 138);
            this.panelTasks.TabIndex = 0;
            // 
            // panelView
            // 
            this.panelView.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom)
                        | System.Windows.Forms.AnchorStyles.Left)
                        | System.Windows.Forms.AnchorStyles.Right)));
            this.panelView.Location = new System.Drawing.Point(0, 106);
            this.panelView.Name = "panelView";
            this.panelView.Size = new System.Drawing.Size(1216, 313);
            this.panelView.TabIndex = 1;
            // 
            // button4
            // 
            this.button4.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.button4.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button4.ForeColor = System.Drawing.Color.White;
            this.button4.GSAutoSize = false;
            this.button4.ImageHover = null;
            this.button4.ImageNormal = null;
            this.button4.ImagePressed = null;
            this.button4.Location = new System.Drawing.Point(495, 53);
            this.button4.Name = "button4";
            this.button4.Size = new System.Drawing.Size(87, 85);
            this.button4.StaySelected = false;
            this.button4.TabIndex = 9;
            this.button4.Text = "Clans";
            this.button4.UseVisualStyleBackColor = true;
            this.button4.Click += new System.EventHandler(this.button4_Click_1);
            // 
            // button7
            // 
            this.button7.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.button7.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button7.ForeColor = System.Drawing.Color.White;
            this.button7.GSAutoSize = false;
            this.button7.ImageHover = global::WOAdmin.Properties.Resources.ADTransactionsOver;
            this.button7.ImageNormal = global::WOAdmin.Properties.Resources.ADTransactionsNormal;
            this.button7.ImagePressed = global::WOAdmin.Properties.Resources.ADTransactionsUp;
            this.button7.Location = new System.Drawing.Point(598, 39);
            this.button7.Name = "button7";
            this.button7.Size = new System.Drawing.Size(108, 99);
            this.button7.StaySelected = false;
            this.button7.TabIndex = 6;
            this.button7.UseVisualStyleBackColor = true;
            this.button7.Click += new System.EventHandler(this.button7_Click);
            // 
            // button6
            // 
            this.button6.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Right)));
            this.button6.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.button6.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button6.ForeColor = System.Drawing.Color.White;
            this.button6.GSAutoSize = false;
            this.button6.ImageHover = global::WOAdmin.Properties.Resources.ADExitOver;
            this.button6.ImageNormal = global::WOAdmin.Properties.Resources.ADExitNormal;
            this.button6.ImagePressed = global::WOAdmin.Properties.Resources.ADExitUp;
            this.button6.Location = new System.Drawing.Point(1096, 39);
            this.button6.Name = "button6";
            this.button6.Size = new System.Drawing.Size(108, 99);
            this.button6.StaySelected = false;
            this.button6.TabIndex = 5;
            this.button6.UseVisualStyleBackColor = true;
            this.button6.Click += new System.EventHandler(this.button6_Click);
            // 
            // button3
            // 
            this.button3.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.button3.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button3.ForeColor = System.Drawing.Color.White;
            this.button3.GSAutoSize = false;
            this.button3.ImageHover = global::WOAdmin.Properties.Resources.ADItemsOver;
            this.button3.ImageNormal = global::WOAdmin.Properties.Resources.ADItemsNormal;
            this.button3.ImagePressed = global::WOAdmin.Properties.Resources.ADItemsUp;
            this.button3.Location = new System.Drawing.Point(370, 39);
            this.button3.Name = "button3";
            this.button3.Size = new System.Drawing.Size(108, 99);
            this.button3.StaySelected = false;
            this.button3.TabIndex = 2;
            this.button3.UseVisualStyleBackColor = true;
            this.button3.Click += new System.EventHandler(this.button3_Click);
            // 
            // button2
            // 
            this.button2.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.button2.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button2.ForeColor = System.Drawing.Color.White;
            this.button2.GSAutoSize = false;
            this.button2.ImageHover = global::WOAdmin.Properties.Resources.ADUsersOver;
            this.button2.ImageNormal = global::WOAdmin.Properties.Resources.ADUsersNormal;
            this.button2.ImagePressed = global::WOAdmin.Properties.Resources.ADUsersUp;
            this.button2.Location = new System.Drawing.Point(123, 39);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(108, 99);
            this.button2.StaySelected = false;
            this.button2.TabIndex = 1;
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // button1
            // 
            this.button1.BackgroundImageLayout = System.Windows.Forms.ImageLayout.Stretch;
            this.button1.Font = new System.Drawing.Font("Arial", 8.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(204)));
            this.button1.ForeColor = System.Drawing.Color.White;
            this.button1.GSAutoSize = false;
            this.button1.ImageHover = global::WOAdmin.Properties.Resources.ADHomeOver;
            this.button1.ImageNormal = global::WOAdmin.Properties.Resources.ADHomeNormal;
            this.button1.ImagePressed = global::WOAdmin.Properties.Resources.ADHomeUp;
            this.button1.Location = new System.Drawing.Point(9, 39);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(108, 99);
            this.button1.StaySelected = false;
            this.button1.TabIndex = 0;
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1216, 419);
            this.Controls.Add(this.panelView);
            this.Controls.Add(this.panelTasks);
            this.Name = "Form1";
            this.Text = "War Inc Admin Dashboard";
            this.Shown += new System.EventHandler(this.Form1_Shown);
            this.panelTasks.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Panel panelTasks;
        private System.Windows.Forms.Panel panelView;
        private WOAdmin.GSButton button6;
        private WOAdmin.GSButton button3;
        private WOAdmin.GSButton button1;
        private WOAdmin.GSButton button7;
        private WOAdmin.GSButton button4;
        private GSButton button2;
    }
}

